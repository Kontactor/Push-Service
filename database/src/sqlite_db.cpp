#include "sqlite_db.h"

#include <stdexcept>

namespace sqlite_db {

Database::Database(const std::string& file_path) {
    sqlite3* raw_db = nullptr;

    int return_code = sqlite3_open(file_path.c_str(), &raw_db);

    if (return_code != SQLITE_OK) {
        const std::string error_msg = sqlite3_errmsg(raw_db);
        throw std::runtime_error("Cannot open database : " + (error_msg.empty() ? "Unknown error" : error_msg));
    }

    db_ = std::shared_ptr<sqlite3>(raw_db, sqlite3_close);
}

std::optional<std::string> Database::GetLastError() const {
    if (!last_error_.empty())
        return last_error_;

    return std::nullopt;
}

}  // namespace sqlite_db
