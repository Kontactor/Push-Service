#pragma once
#include <memory>
#include <optional>
#include <sqlite3.h>
#include <string>
#include <vector>

namespace sqlite_db {
using Row = std::vector<std::string>;
using QueryResult = std::vector<Row>;

class Database {
public:
    explicit Database(const std::string& file_path);

    std::optional<std::string> GetLastError() const;
    sqlite3* Handle() {
        return db_.get();
    }

    // INSERT / UPDATE / DELETE
    template <typename... Params>
    bool ExecuteNonQuery(const std::string& query, Params&&... params) {
        sqlite3_stmt* stmt = nullptr;
        const char* error_msg;

        int return_code = sqlite3_prepare_v2(db_.get(), query.c_str(), -1, &stmt, nullptr);
        if (return_code != SQLITE_OK) {
            error_msg = sqlite3_errmsg(db_.get());
            last_error_ = error_msg ? error_msg : "Unknown error";

            return false;
        }

        BindParams(stmt, 1, std::forward<Params>(params)...);

        return_code = sqlite3_step(stmt);
        if (return_code != SQLITE_DONE) {
            error_msg = sqlite3_errmsg(db_.get());
            last_error_ = error_msg ? error_msg : "Unknown error";
            sqlite3_finalize(stmt);

            return false;
        }

        sqlite3_finalize(stmt);

        return true;
    }

    // SELECT
    template <typename... Params>
    std::optional<QueryResult> ExecuteQuery(const std::string& query, Params&&... params) {
        sqlite3_stmt* stmt = nullptr;
        const char* error_msg;

        int return_code = sqlite3_prepare_v2(db_.get(), query.c_str(), -1, &stmt, nullptr);
        if (return_code != SQLITE_OK) {
            error_msg = sqlite3_errmsg(db_.get());
            last_error_ = error_msg ? error_msg : "Unknown error";

            return std::nullopt;
        }

        BindParams(stmt, 1, std::forward<Params>(params)...);

        QueryResult result;

        while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
            int col_count = sqlite3_column_count(stmt);
            Row row;
            row.reserve(col_count);

            for (int i = 0; i < col_count; ++i) {
                auto text = sqlite3_column_text(stmt, i);
                row.emplace_back(text ? reinterpret_cast<const char*>(text) : "");
            }

            result.emplace_back(std::move(row));
        }

        if (return_code != SQLITE_DONE) {
            last_error_ = sqlite3_errmsg(db_.get());
            sqlite3_finalize(stmt);
            return std::nullopt;
        }

        sqlite3_finalize(stmt);
        return result;
    }

private:
    std::shared_ptr<sqlite3> db_;
    std::string last_error_;

    template <typename... Params>
    void BindParams(sqlite3_stmt* stmt, int start_index, Params&&... params) {
        int index = start_index;
        (BindParam(stmt, index++, std::forward<Params>(params)), ...);
    }

    void BindParam(sqlite3_stmt* stmt, int index, int value) {
        sqlite3_bind_int(stmt, index, value);
    }

    void BindParam(sqlite3_stmt* stmt, int index, const std::string& value) {
        sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
    }

    // TODO привязку остальных типов
};

}  // namespace sqlite_db
