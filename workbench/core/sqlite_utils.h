#pragma once
#include <cstdint>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
namespace UFlipper {
namespace Detail {
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

} // namespace Detail
} // namespace UFlipper
