#include "capture_store.h"
#include "io_utils.h"
#include <array>
#include <chrono>
#include <cstring>
#include <map>
#include <sqlite3.h>
#include <sys/file.h>
namespace UFlipper {
namespace fs = std::filesystem;
const char *stateName(CaptureState state) {
    switch (state) {
    case CaptureState::Recording:
        return "recording";
    case CaptureState::Completed:
        return "completed";
    case CaptureState::Interrupted:
        return "interrupted";
    }
    return "interrupted";
}
namespace {
struct FileDescriptor {
    int value = -1;
    ~FileDescriptor() {
        if (value >= 0)
            ::close(value);
    }
};
struct Database {
    sqlite3 *value = nullptr;
    ~Database() {
        if (value)
            sqlite3_close(value);
    }
    void execute(const char *sql) {
        char *error = nullptr;
        if (sqlite3_exec(value, sql, nullptr, nullptr, &error) != SQLITE_OK) {
            std::string message = error ? error : "Database error";
            sqlite3_free(error);
            throw std::runtime_error(message);
        }
    }
};
class Statement {
  public:
    Statement(sqlite3 *database, const char *sql) {
        if (sqlite3_prepare_v2(database, sql, -1, &value_, nullptr) != SQLITE_OK)
            throw std::runtime_error(sqlite3_errmsg(database));
    }
    ~Statement() {
        sqlite3_finalize(value_);
    }
    void bind(int index, const std::string &text) {
        if (sqlite3_bind_text(value_, index, text.c_str(), static_cast<int>(text.size()), SQLITE_TRANSIENT) !=
            SQLITE_OK)
            throw std::runtime_error("Cannot bind catalog field");
    }
    void number(int index, sqlite3_int64 number) {
        if (sqlite3_bind_int64(value_, index, number) != SQLITE_OK)
            throw std::runtime_error("Cannot bind catalog number");
    }
    bool next() {
        auto state = sqlite3_step(value_);
        if (state == SQLITE_ROW)
            return true;
        if (state != SQLITE_DONE)
            throw std::runtime_error(sqlite3_errmsg(sqlite3_db_handle(value_)));
        return false;
    }
    std::string text(int column) const {
        const auto *t = sqlite3_column_text(value_, column);
        return t ? reinterpret_cast<const char *>(t) : "";
    }
    uint64_t number(int column) const {
        return static_cast<uint64_t>(sqlite3_column_int64(value_, column));
    }

  private:
    sqlite3_stmt *value_ = nullptr;
};
uint64_t utcMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}
Capture readCapture(Statement &query) {
    Capture c;
    c.id = query.text(0);
    c.kind = query.text(1);
    c.metadata = query.text(2);
    auto state = query.text(3);
    c.state = state == "recording"   ? CaptureState::Recording
              : state == "completed" ? CaptureState::Completed
                                     : CaptureState::Interrupted;
    c.bytes = query.number(4);
    c.reason = query.text(5);
    c.sha256 = query.text(6);
    return c;
}
bool validId(const std::string &id) {
    return id.size() == 32 && id.find_first_not_of("0123456789abcdef") == std::string::npos;
}
bool validRecordFile(const fs::path &path) {
    std::ifstream file(path, std::ios::binary);
    char magic[8];
    if (!file.read(magic, 8) || std::memcmp(magic, "UFCAP001", 8) != 0)
        return false;
    while (file.peek() != std::char_traits<char>::eof()) {
        unsigned char header[13];
        if (!file.read(reinterpret_cast<char *>(header), sizeof header) || header[0] > 2)
            return false;
        uint32_t length = uint32_t(header[9]) | (uint32_t(header[10]) << 8) | (uint32_t(header[11]) << 16) |
                          (uint32_t(header[12]) << 24);
        if (length > 1024 * 1024)
            return false;
        file.ignore(length);
        if (file.gcount() != length)
            return false;
    }
    return !file.bad();
}
} // namespace
struct CaptureStore::Impl {
    fs::path root;
    FileDescriptor lease;
    Database database;
    std::map<std::string, std::unique_ptr<FileDescriptor>> active;
    explicit Impl(const fs::path &path) : root(path) {
        initializeSodium();
        bool existed = fs::exists(root);
        fs::create_directories(root);
        if (!existed)
            fs::permissions(root, fs::perms::owner_all);
        lease.value = ::open((root / "catalog.lock").c_str(), O_CREAT | O_RDWR, 0600);
        if (lease.value < 0 || flock(lease.value, LOCK_EX | LOCK_NB) != 0)
            throw std::runtime_error("Another uFlipper instance owns the recording catalog");
        if (sqlite3_open((root / "catalog.sqlite").c_str(), &database.value) != SQLITE_OK)
            throw std::runtime_error("Cannot open recording catalog");
        database.execute("PRAGMA journal_mode=WAL; PRAGMA synchronous=FULL; PRAGMA foreign_keys=ON;");
        database.execute(
            "CREATE TABLE IF NOT EXISTS recordings(id TEXT PRIMARY KEY,kind TEXT NOT NULL,metadata TEXT NOT "
            "NULL,state TEXT NOT NULL,bytes INTEGER NOT NULL DEFAULT 0,reason TEXT NOT NULL DEFAULT "
            "'',sha256 TEXT NOT NULL DEFAULT '',started_ms INTEGER NOT NULL,ended_ms INTEGER);");
        fs::create_directories(root / "recordings");
    }
    fs::path folder(const std::string &id) const {
        if (!validId(id))
            throw std::invalid_argument("Invalid recording ID");
        return root / "recordings" / id;
    }
    Capture get(const std::string &id) const {
        Statement query(database.value,
                        "SELECT id,kind,metadata,state,bytes,reason,sha256 FROM recordings WHERE id=?");
        query.bind(1, id);
        if (!query.next())
            throw std::out_of_range("Recording not found");
        return readCapture(query);
    }
    void manifest(const Capture &item) const {
        std::string json =
            "{\n\"format\":1,\"id\":" + jsonQuote(item.id) + ",\"kind\":" + jsonQuote(item.kind) +
            ",\"state\":" + jsonQuote(stateName(item.state)) + ",\"bytes\":" + std::to_string(item.bytes) +
            ",\"sha256\":" + jsonQuote(item.sha256) + ",\"metadata\":" + jsonQuote(item.metadata) +
            ",\"reason\":" + jsonQuote(item.reason) + ",\"timebase\":\"host_monotonic_nanoseconds\"\n}\n";
        AtomicOutput file(folder(item.id) / "manifest.json");
        writeAll(file.fd(), json.data(), json.size());
        file.publish(true);
    }
    void finalize(const std::string &id, CaptureState state, const std::string &reason) {
        auto path = folder(id) / "events.bin";
        std::error_code error;
        auto bytes = fs::file_size(path, error);
        auto hash = error ? std::string() : hashFile(path);
        Statement update(database.value,
                         "UPDATE recordings SET state=?,bytes=?,reason=?,sha256=?,ended_ms=? WHERE id=?");
        update.bind(1, stateName(state));
        update.number(2, error ? 0 : bytes);
        update.bind(3, reason);
        update.bind(4, hash);
        update.number(5, utcMilliseconds());
        update.bind(6, id);
        update.next();
        manifest(get(id));
    }
};
CaptureStore::CaptureStore(const fs::path &root) : impl_(new Impl(root)) {
    Statement query(impl_->database.value, "SELECT id FROM recordings WHERE state='recording'");
    std::vector<std::string> interrupted;
    while (query.next())
        interrupted.push_back(query.text(0));
    for (const auto &id : interrupted)
        impl_->finalize(id, CaptureState::Interrupted,
                        "Recovered after an interrupted process; trailing records may be incomplete");
}
CaptureStore::~CaptureStore() {
    std::vector<std::string> ids;
    for (const auto &pair : impl_->active)
        ids.push_back(pair.first);
    for (const auto &id : ids)
        try {
            finish(id, CaptureState::Interrupted, "Application closed before device stop was confirmed");
        } catch (...) {
        }
}
std::string CaptureStore::begin(const std::string &kind, const std::string &metadata) {
    if (kind.empty() || metadata.size() > 1024 * 1024)
        throw std::invalid_argument("Invalid recording metadata");
    if (impl_->active.size() >= 32)
        throw std::runtime_error("Too many active recordings");
    auto id = randomId();
    fs::create_directory(impl_->folder(id));
    fs::permissions(impl_->folder(id), fs::perms::owner_all);
    auto file = std::make_unique<FileDescriptor>();
    file->value = ::open((impl_->folder(id) / "events.bin").c_str(), O_CREAT | O_WRONLY | O_EXCL, 0600);
    if (file->value < 0)
        throw std::runtime_error("Cannot create recording file");
    writeAll(file->value, "UFCAP001", 8);
    syncFile(file->value);
    Statement insert(impl_->database.value,
                     "INSERT INTO recordings(id,kind,metadata,state,started_ms) VALUES(?,?,?,'recording',?)");
    insert.bind(1, id);
    insert.bind(2, kind);
    insert.bind(3, metadata);
    insert.number(4, utcMilliseconds());
    insert.next();
    impl_->active.emplace(id, std::move(file));
    impl_->manifest(impl_->get(id));
    return id;
}
void CaptureStore::append(const std::string &id, const void *bytes, size_t size, uint64_t time,
                          RecordType type) {
    if (size > 1024 * 1024 || (!bytes && size) || static_cast<unsigned char>(type) > 2)
        throw std::invalid_argument("Recording frame exceeds format limits");
    auto found = impl_->active.find(id);
    if (found == impl_->active.end())
        throw std::out_of_range("Recording is not active");
    unsigned char header[13] = {static_cast<unsigned char>(type)};
    for (unsigned i = 0; i < 8; i++)
        header[1 + i] = static_cast<unsigned char>(time >> (i * 8));
    for (unsigned i = 0; i < 4; i++)
        header[9 + i] = static_cast<unsigned char>(size >> (i * 8));
    writeAll(found->second->value, header, sizeof header);
    writeAll(found->second->value, bytes, size);
}
void CaptureStore::finish(const std::string &id, CaptureState state, const std::string &reason) {
    if (state == CaptureState::Recording)
        throw std::invalid_argument("Final recording state is required");
    auto found = impl_->active.find(id);
    if (found == impl_->active.end())
        throw std::out_of_range("Recording is not active");
    syncFile(found->second->value);
    impl_->active.erase(found);
    impl_->finalize(id, state, reason);
}
std::vector<Capture> CaptureStore::list(size_t limit, size_t offset) const {
    if (limit > 10000)
        throw std::invalid_argument("Catalog page too large");
    Statement query(impl_->database.value, "SELECT id,kind,metadata,state,bytes,reason,sha256 FROM "
                                           "recordings ORDER BY started_ms DESC,id LIMIT ? OFFSET ?");
    query.number(1, limit);
    query.number(2, offset);
    std::vector<Capture> items;
    while (query.next())
        items.push_back(readCapture(query));
    return items;
}
bool CaptureStore::verify(const std::string &id) const {
    auto item = impl_->get(id);
    if (item.state == CaptureState::Recording || item.sha256.empty())
        return false;
    auto path = recordingPath(id);
    try {
        return fs::file_size(path) == item.bytes && hashFile(path) == item.sha256 && validRecordFile(path);
    } catch (const std::exception &) {
        return false;
    }
}
fs::path CaptureStore::recordingPath(const std::string &id) const {
    return impl_->folder(id) / "events.bin";
}
} // namespace UFlipper
