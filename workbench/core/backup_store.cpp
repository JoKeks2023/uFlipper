#include "backup_store.h"
#include "io_utils.h"
#include "sqlite_utils.h"
#include <chrono>
#include <functional>
#include <limits>
#include <sys/file.h>
namespace UFlipper {
namespace fs = std::filesystem;
using Detail::Database;
using Detail::Statement;
namespace {
uint64_t utcMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}
bool validId(const std::string &id) {
    return id.size() == 32 && id.find_first_not_of("0123456789abcdef") == std::string::npos;
}
bool validPath(const std::string &path) {
    if (path.size() > 4096 || path.find('\0') != std::string::npos)
        return false;
    if (path != "/int" && path != "/ext" && path.rfind("/int/", 0) != 0 && path.rfind("/ext/", 0) != 0)
        return false;
    size_t start = 1;
    while (start < path.size()) {
        auto end = path.find('/', start);
        auto part = path.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (part.empty() || part == "." || part == "..")
            return false;
        if (end == std::string::npos)
            return true;
        start = end + 1;
    }
    return false;
}
struct Descriptor {
    int fd = -1;
    ~Descriptor() {
        if (fd >= 0)
            ::close(fd);
    }
};
struct ActiveFile {
    Descriptor file;
    std::string backup, path, blob;
    uint64_t expected = 0, written = 0;
    crypto_hash_sha256_state digest;
};
BackupInfo readInfo(Statement &row) {
    BackupInfo info;
    info.startedMs = row.number(5);
    info.endedMs = row.number(6);
    info.id = row.text(0);
    info.metadata = row.text(1);
    info.reason = row.text(2);
    info.scope = row.text(3) == "full" ? BackupScope::FullDevice : BackupScope::TestSubset;
    const auto state = row.text(4);
    info.state = state == "verified"  ? BackupState::Verified
                 : state == "writing" ? BackupState::Writing
                                      : BackupState::Interrupted;
    return info;
}
BackupEntry readEntry(Statement &row) {
    return {row.text(0), row.text(1), row.text(2), row.number(3), row.text(4) == "directory"};
}
} // namespace
struct BackupStore::Impl {
    fs::path root;
    Descriptor lease;
    Database database;
    std::string active;
    std::unique_ptr<ActiveFile> file;
    explicit Impl(const fs::path &workbench) : root(workbench / "backups") {
        fs::create_directories(root);
        fs::permissions(root, fs::perms::owner_all);
        lease.fd = ::open((root / "catalog.lock").c_str(), O_CREAT | O_RDWR, 0600);
        if (lease.fd < 0 || flock(lease.fd, LOCK_EX | LOCK_NB) != 0)
            throw std::runtime_error("Another owner holds the backup catalog");
        if (sqlite3_open((root / "catalog.sqlite").c_str(), &database.value) != SQLITE_OK)
            throw std::runtime_error("Cannot open backup catalog");
        database.execute("PRAGMA journal_mode=WAL; PRAGMA synchronous=FULL; PRAGMA foreign_keys=ON;");
        database.execute(
            "CREATE TABLE IF NOT EXISTS backups(id TEXT PRIMARY KEY,metadata TEXT NOT NULL,reason TEXT NOT "
            "NULL DEFAULT '',scope TEXT NOT NULL,state TEXT NOT NULL,inventory INTEGER NOT NULL DEFAULT "
            "0,manifest_sha256 TEXT NOT NULL DEFAULT '',started_ms INTEGER NOT NULL,ended_ms INTEGER NOT "
            "NULL DEFAULT 0);");
        database.execute(
            "CREATE TABLE IF NOT EXISTS entries(backup TEXT NOT NULL REFERENCES backups(id),path TEXT NOT "
            "NULL,kind TEXT NOT NULL,blob TEXT NOT NULL DEFAULT '',bytes INTEGER NOT NULL DEFAULT 0,sha256 "
            "TEXT NOT NULL DEFAULT '',ready INTEGER NOT NULL DEFAULT 0,PRIMARY KEY(backup,path));");
        database.execute(
            "UPDATE backups SET state='interrupted',reason='Process interrupted before backup "
            "verification',ended_ms=CAST(strftime('%s','now') AS INTEGER)*1000 WHERE state='writing';");
    }
    fs::path folder(const std::string &id) const {
        if (!validId(id))
            throw std::invalid_argument("Invalid backup ID");
        return root / id;
    }
    void requireActive(const std::string &id) const {
        if (active != id)
            throw std::runtime_error("Backup is not active");
    }
    void insertEntry(const std::string &id, const std::string &path, const std::string &kind,
                     const std::string &blob, uint64_t bytes, bool ready) {
        requireActive(id);
        if (!validPath(path) || bytes > uint64_t(std::numeric_limits<int64_t>::max()))
            throw std::invalid_argument("Unsafe backup path or size");
        Statement insert(database.value,
                         "INSERT INTO entries(backup,path,kind,blob,bytes,ready) VALUES(?,?,?,?,?,?)");
        insert.bind(1, id);
        insert.bind(2, path);
        insert.bind(3, kind);
        insert.bind(4, blob);
        insert.number(5, bytes);
        insert.number(6, ready);
        insert.next();
        Statement invalidate(database.value, "UPDATE backups SET inventory=inventory & ? WHERE id=?");
        invalidate.number(1, path == "/int" || path.rfind("/int/", 0) == 0 ? 2 : 1);
        invalidate.bind(2, id);
        invalidate.next();
    }
    void writeManifest(const std::string &id, const BackupInfo &info,
                       const std::function<void(const std::string &)> &emit) const {
        emit(
            "{\"format\":1,\"id\":" + jsonQuote(id) +
            ",\"scope\":" + jsonQuote(info.scope == BackupScope::FullDevice ? "full-device" : "test-subset") +
            ",\"metadata\":" + jsonQuote(info.metadata) + ",\"startedMs\":" + std::to_string(info.startedMs) +
            ",\"endedMs\":" + std::to_string(info.endedMs) + ",\"entries\":[\n");
        Statement rows(database.value,
                       "SELECT path,sha256,blob,bytes,kind FROM entries WHERE backup=? ORDER BY path");
        rows.bind(1, id);
        bool first = true;
        while (rows.next()) {
            auto entry = readEntry(rows);
            if (!first)
                emit(",\n");
            first = false;
            emit("{\"path\":" + jsonQuote(entry.path) + ",\"directory\":" +
                 (entry.directory ? "true" : "false") + ",\"bytes\":" + std::to_string(entry.bytes) +
                 ",\"sha256\":" + jsonQuote(entry.sha256) + ",\"blob\":" + jsonQuote(entry.blob) + "}");
        }
        emit("\n]}\n");
    }
    bool filesValid(const std::string &id) const {
        Statement rows(database.value,
                       "SELECT path,sha256,blob,bytes,kind,ready FROM entries WHERE backup=? ORDER BY path");
        rows.bind(1, id);
        while (rows.next()) {
            auto entry = readEntry(rows);
            if (!rows.number(5) || !validPath(entry.path))
                return false;
            if (entry.directory)
                continue;
            if (!validId(entry.blob) || entry.sha256.size() != 64)
                return false;
            auto path = folder(id) / "payload" / entry.blob;
            if (fs::symlink_status(path).type() != fs::file_type::regular ||
                fs::file_size(path) != entry.bytes || hashFile(path) != entry.sha256)
                return false;
        }
        return true;
    }
};
BackupStore::BackupStore(const fs::path &root) : impl_(new Impl(root)) {}
BackupStore::~BackupStore() {
    if (!impl_->active.empty())
        try {
            interrupt(impl_->active, "Application closed during backup");
        } catch (...) {
        }
}
std::string BackupStore::begin(BackupScope scope, const std::string &metadata) {
    if (!impl_->active.empty())
        throw std::runtime_error("Another backup is active");
    if (metadata.size() > 1024 * 1024)
        throw std::invalid_argument("Backup metadata too large");
    auto id = randomId();
    fs::create_directories(impl_->folder(id) / "payload");
    fs::permissions(impl_->folder(id), fs::perms::owner_all);
    Statement insert(impl_->database.value,
                     "INSERT INTO backups(id,metadata,scope,state,started_ms) VALUES(?,?,?,'writing',?)");
    insert.bind(1, id);
    insert.bind(2, metadata);
    insert.bind(3, scope == BackupScope::FullDevice ? "full" : "test-subset");
    insert.number(4, utcMs());
    insert.next();
    impl_->active = id;
    return id;
}
void BackupStore::addDirectory(const std::string &id, const std::string &path) {
    impl_->insertEntry(id, path, "directory", "", 0, true);
}
void BackupStore::beginFile(const std::string &id, const std::string &path, uint64_t expected) {
    impl_->requireActive(id);
    if (impl_->file)
        throw std::runtime_error("Previous backup file is still open");
    if (path == "/int" || path == "/ext" || !validPath(path))
        throw std::invalid_argument("Unsafe backup file path");
    auto file = std::make_unique<ActiveFile>();
    file->backup = id;
    file->path = path;
    file->blob = randomId();
    file->expected = expected;
    crypto_hash_sha256_init(&file->digest);
    auto target = impl_->folder(id) / "payload" / file->blob;
    file->file.fd = ::open(target.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (file->file.fd < 0)
        throw std::runtime_error("Cannot create backup payload");
    try {
        impl_->insertEntry(id, path, "file", file->blob, expected, false);
    } catch (...) {
        fs::remove(target);
        throw;
    }
    impl_->file = std::move(file);
}
void BackupStore::appendFile(const std::string &id, const void *bytes, size_t size) {
    impl_->requireActive(id);
    auto &file = impl_->file;
    if (!file || (!bytes && size) || size > 1024 * 1024 || size > file->expected - file->written)
        throw std::invalid_argument("Invalid backup chunk or file size");
    writeAll(file->file.fd, bytes, size);
    if (size) crypto_hash_sha256_update(&file->digest, static_cast<const unsigned char *>(bytes), size);
    file->written += size;
}
void BackupStore::finishFile(const std::string &id) {
    impl_->requireActive(id);
    auto &file = impl_->file;
    if (!file || file->written != file->expected)
        throw std::runtime_error("Backup file is incomplete");
    syncFile(file->file.fd);
    unsigned char digest[crypto_hash_sha256_BYTES];
    char hash[crypto_hash_sha256_BYTES * 2 + 1];
    crypto_hash_sha256_final(&file->digest, digest);
    sodium_bin2hex(hash, sizeof hash, digest, sizeof digest);
    // The final verification rereads payloads on a worker; finishing each streamed
    // file must not reread a large file on the desktop UI thread.

    Statement update(impl_->database.value, "UPDATE entries SET ready=1,sha256=? WHERE backup=? AND path=?");
    update.bind(1, hash);
    update.bind(2, id);
    update.bind(3, file->path);
    update.next();
    file.reset();
}
void BackupStore::confirmInventory(const std::string &id, const std::string &root) {
    impl_->requireActive(id);
    if (root != "/int" && root != "/ext")
        throw std::invalid_argument("Unknown backup storage root");
    if (impl_->file)
        throw std::runtime_error("Cannot confirm inventory while a backup file is open");
    Statement rootEntry(
        impl_->database.value,
        "SELECT COUNT(*) FROM entries WHERE backup=? AND path=? AND kind='directory' AND ready=1");
    rootEntry.bind(1, id);
    rootEntry.bind(2, root);
    rootEntry.next();
    if (rootEntry.number(0) != 1)
        throw std::runtime_error("Storage root was not inventoried");
    Statement update(impl_->database.value, "UPDATE backups SET inventory=inventory|? WHERE id=?");
    update.number(1, root == "/int" ? 1 : 2);
    update.bind(2, id);
    update.next();
}
void BackupStore::finish(const std::string &id) {
    impl_->requireActive(id);
    if (impl_->file)
        throw std::runtime_error("Backup file still open");
    Statement endTime(impl_->database.value, "UPDATE backups SET ended_ms=? WHERE id=?");
    endTime.number(1, utcMs());
    endTime.bind(2, id);
    endTime.next();
    auto info = get(id);
    Statement inventory(impl_->database.value, "SELECT inventory FROM backups WHERE id=?");
    inventory.bind(1, id);
    inventory.next();
    if (info.scope == BackupScope::FullDevice && inventory.number(0) != 3)
        throw std::runtime_error("Full backup requires confirmed internal and SD inventories");
    if (!impl_->filesValid(id))
        throw std::runtime_error("Backup payload verification failed");
    AtomicOutput manifest(impl_->folder(id) / "manifest.json");
    impl_->writeManifest(
        id, info, [&](const std::string &bytes) { writeAll(manifest.fd(), bytes.data(), bytes.size()); });
    manifest.publish();
    auto hash = hashFile(impl_->folder(id) / "manifest.json");
    Statement update(impl_->database.value,
                     "UPDATE backups SET state='verified',manifest_sha256=? WHERE id=?");
    update.bind(1, hash);
    update.bind(2, id);
    update.next();
    impl_->active.clear();
}
void BackupStore::interrupt(const std::string &id, const std::string &reason) {
    impl_->requireActive(id);
    impl_->file.reset();
    Statement update(impl_->database.value,
                     "UPDATE backups SET state='interrupted',reason=?,ended_ms=? WHERE id=?");
    update.bind(1, reason);
    update.number(2, utcMs());
    update.bind(3, id);
    update.next();
    impl_->active.clear();
}
BackupInfo BackupStore::get(const std::string &id) const {
    impl_->folder(id);
    Statement query(impl_->database.value,
                    "SELECT id,metadata,reason,scope,state,started_ms,ended_ms FROM backups WHERE id=?");
    query.bind(1, id);
    if (!query.next())
        throw std::out_of_range("Backup not found");
    return readInfo(query);
}
bool BackupStore::verify(const std::string &id) const {
    try {
        if (get(id).state != BackupState::Verified)
            return false;
        Statement query(impl_->database.value, "SELECT manifest_sha256 FROM backups WHERE id=?");
        query.bind(1, id);
        query.next();
        const auto expected = query.text(0);
        crypto_hash_sha256_state state;
        crypto_hash_sha256_init(&state);
        impl_->writeManifest(id, get(id), [&](const std::string &bytes) {
            crypto_hash_sha256_update(&state, reinterpret_cast<const unsigned char *>(bytes.data()),
                                      bytes.size());
        });
        unsigned char digest[32];
        char hex[65];
        crypto_hash_sha256_final(&state, digest);
        sodium_bin2hex(hex, sizeof hex, digest, sizeof digest);
        return !expected.empty() && expected == hex &&
               hashFile(impl_->folder(id) / "manifest.json") == expected && impl_->filesValid(id);
    } catch (const std::exception &) {
        return false;
    }
}
std::vector<BackupInfo> BackupStore::list(size_t limit, size_t offset) const {
    if (limit > 10000)
        throw std::invalid_argument("Backup page too large");
    Statement query(impl_->database.value, "SELECT id,metadata,reason,scope,state,started_ms,ended_ms FROM "
                                           "backups ORDER BY rowid DESC LIMIT ? OFFSET ?");
    query.number(1, limit);
    query.number(2, offset);
    std::vector<BackupInfo> result;
    while (query.next())
        result.push_back(readInfo(query));
    return result;
}
std::vector<BackupEntry> BackupStore::entries(const std::string &id, size_t limit, size_t offset) const {
    impl_->folder(id);
    if (limit > 10000)
        throw std::invalid_argument("Backup entry page too large");
    Statement query(
        impl_->database.value,
        "SELECT path,sha256,blob,bytes,kind FROM entries WHERE backup=? ORDER BY path LIMIT ? OFFSET ?");
    query.bind(1, id);
    query.number(2, limit);
    query.number(3, offset);
    std::vector<BackupEntry> result;
    while (query.next())
        result.push_back(readEntry(query));
    return result;
}
fs::path BackupStore::payloadPath(const std::string &id, const BackupEntry &entry) const {
    if (!validId(entry.blob) || entry.directory || !validPath(entry.path))
        throw std::invalid_argument("Invalid backup file entry");
    return impl_->folder(id) / "payload" / entry.blob;
}
} // namespace UFlipper
