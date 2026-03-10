#include "push_storage.h"

#include <chrono>
#include <format>
#include <sqlite3.h>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace push_storage {
namespace {
using namespace std::literals;

// tabs names and keys for unordered_maps with sql strings that return by the
// functions GetSqlCreateTabQueries and GetSqlFillTabQueries
constexpr std::string_view kPushEventsTableKey = "PushEvents";
constexpr std::string_view kTypeIdTableKey = "TypeId";
constexpr std::string_view kPriorityIdTableKey = "PriorityId";
constexpr std::string_view kSenderIdTableKey = "SenderId";

// PushEvents tab column names
constexpr std::string_view TYPE_ID = "type_id";
constexpr std::string_view PRIORITY_ID = "priority_id";
constexpr std::string_view SENDER_ID = "sender_id";
constexpr std::string_view PAYLOAD = "payload";
constexpr std::string_view TIMESTAMP = "timestamp";

// TypeId tab column names
constexpr std::string_view TYPE_DESCRIPTION = "type_description";
// PriorityId tab column names
constexpr std::string_view PRIORIY_DESCRIPTION = "priority_description";
// Sender tab column names
constexpr std::string_view SENDER_DESCRIPTION = "sender_description";

// Certain descriptions
constexpr std::string_view TYPE1_DESCRIPTION = "Push type 1";
constexpr std::string_view TYPE2_DESCRIPTION = "Push type 2";
constexpr std::string_view NOTIFICTION_DESCRIPTION = "Push that notificate";
constexpr std::string_view WARNING_DESCRIPTION = "Push that warning";
constexpr std::string_view ALERT_DESCRIPTION = "Push that alert";
constexpr std::string_view SENDER0 = "Test";
constexpr std::string_view SENDER1 = "Admin";

// Now only two senders
constexpr int TEST_SENDER_ID = 0;
constexpr int ADMIN_SENDER_ID = 1;

// return unordered_map with pair (table name - sql-query to create current tab)
std::unordered_map<std::string_view, std::string> const GetSqlCreateTabQueries() {
    std::unordered_map<std::string_view, std::string> sql_strings;
    sql_strings[kPushEventsTableKey] = std::format(
        "CREATE TABLE {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "{} TINYINT NOT NULL, "
        "{} TINYINT NOT NULL, "
        "{} TEXT NOT NULL, "
        "{} INTEGER NOT NULL, "
        "{} TEXT NOT NULL);",
        kPushEventsTableKey,
        TYPE_ID,
        PRIORITY_ID,
        PAYLOAD,
        SENDER_ID,
        TIMESTAMP);
    sql_strings[kTypeIdTableKey] = std::format(
        "CREATE TABLE {} ("
        "{} TINYINT PRIMARY KEY NOT NULL, "
        "{} TEXT NOT NULL);",
        kTypeIdTableKey,
        TYPE_ID,
        TYPE_DESCRIPTION);
    sql_strings[kPriorityIdTableKey] = std::format(
        "CREATE TABLE {} ("
        "{} TINYINT PRIMARY KEY NOT NULL, "
        "{} TEXT NOT NULL);",
        kPriorityIdTableKey,
        PRIORITY_ID,
        PRIORIY_DESCRIPTION);
    sql_strings[kSenderIdTableKey] = std::format(
        "CREATE TABLE {} ("
        "{} INTEGER PRIMARY KEY NOT NULL, "
        "{} TEXT NOT NULL);",
        kSenderIdTableKey,
        SENDER_ID,
        SENDER_DESCRIPTION);
    return sql_strings;
}

// return unordered_map with pair (table name - sql-query to fill current tab)
std::unordered_map<std::string_view, std::string> const GetSqlFillTabQueries() {
    std::unordered_map<std::string_view, std::string> sql_strings;
    sql_strings[kTypeIdTableKey] = std::format("INSERT INTO {} ({}, {}) VALUES ({}, '{}'), ({}, '{}');",
                                               kTypeIdTableKey,
                                               TYPE_ID,
                                               TYPE_DESCRIPTION,
                                               static_cast<int>(PushType::Type1),
                                               TYPE1_DESCRIPTION,
                                               static_cast<int>(PushType::Type2),
                                               TYPE2_DESCRIPTION);
    sql_strings[kPriorityIdTableKey] = std::format("INSERT INTO {} ({}, {}) VALUES ({}, '{}'), ({}, '{}'), ({}, '{}');",
                                                   kPriorityIdTableKey,
                                                   PRIORITY_ID,
                                                   PRIORIY_DESCRIPTION,
                                                   static_cast<int>(PushPriority::Notification),
                                                   NOTIFICTION_DESCRIPTION,
                                                   static_cast<int>(PushPriority::Warning),
                                                   WARNING_DESCRIPTION,
                                                   static_cast<int>(PushPriority::Alert),
                                                   ALERT_DESCRIPTION);
    sql_strings[kSenderIdTableKey] = std::format("INSERT INTO {} ({}, {}) VALUES('{}', '{}'), ({}, '{}');",
                                                 kSenderIdTableKey,
                                                 SENDER_ID,
                                                 SENDER_DESCRIPTION,
                                                 TEST_SENDER_ID,
                                                 SENDER0,
                                                 ADMIN_SENDER_ID,
                                                 SENDER1);
    return sql_strings;
}

// returns a string with escaped single quotes
std::string ScreenQuote(const std::string& str) {
    const char quote = '\'';
    const std::string double_quote = "''";

    size_t quote_pos = str.find(quote);
    // if no single quote return original str
    if (quote_pos == str.npos) {
        return str;
    }

    std::string res{};
    res.reserve(str.size() + 1);
    size_t prev_quote_pos = 0;

    do {
        res += str.substr(prev_quote_pos, quote_pos - prev_quote_pos);
        res += quote;
        prev_quote_pos = quote_pos;
        quote_pos = str.find(quote, quote_pos + 1);

    } while (quote_pos != str.npos);

    res += str.substr(prev_quote_pos);
    return res;
}

std::string GetCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_info;

// платформазависимость
#ifdef _WIN32
    localtime_s(&tm_info, &time_t);
#else
    localtime_r(&time_t, &tm_info);
#endif

    std::stringstream ss;
    ss << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
}  // namespace

// class that open or create database, insert push logs to it
class PushStorage : public IPushStorage {
public:
    explicit PushStorage(const std::string& db_name) : db_name_{db_name} {}

    bool OpenDatabase() override;

    bool InsertPush(int sender_id, const IPush& push) override;

    std::optional<std::string> GetLastError() const override;

    ~PushStorage();

private:
    bool ExecSqlQuery(const std::string& sql_query);
    // create tabs and fill with init info
    bool InitTabs();
    // create tabs
    bool CreateTabs();
    // fill tabs with information
    bool FillTabs();
    // checks if the table exists
    bool IsTableExists(const std::string_view& tableName);

    // holds an error message if an error with database occurs
    std::optional<std::string> last_error_{};
    sqlite3* db_;
    std::string db_name_;
};

// open database and
// if tables doesn't exist create them and fill with init info
bool PushStorage::OpenDatabase() {
    if (sqlite3_open(db_name_.c_str(), &db_)) {
        last_error_ = sqlite3_errmsg(db_);
        return false;
    }
    if (!IsTableExists(kPushEventsTableKey)) {
        return InitTabs();
    }
    return true;
}

bool PushStorage::InsertPush(int sender_id, const IPush& push) {
    using namespace std::literals;
    const std::string sql_insert = std::format("INSERT INTO {} ({}, {}, {}, {}, {}) VALUES({}, {}, '{}', '{}', '{}');",
                                               kPushEventsTableKey,
                                               TYPE_ID,
                                               PRIORITY_ID,
                                               PAYLOAD,
                                               SENDER_ID,
                                               TIMESTAMP,
                                               static_cast<int>(push.type),
                                               static_cast<int>(push.priority),
                                               ScreenQuote(push.payload),
                                               sender_id,
                                               GetCurrentTime());
    return ExecSqlQuery(sql_insert);
}

std::optional<std::string> PushStorage::GetLastError() const {
    return last_error_;
}

PushStorage::~PushStorage() {
    sqlite3_close(db_);
}

bool PushStorage::ExecSqlQuery(const std::string& sql_query) {
    char* error_msg;
    if (sqlite3_exec(db_, sql_query.c_str(), nullptr, nullptr, &error_msg) != SQLITE_OK) {
        last_error_ = std::move(error_msg);
        return false;
    }
    return true;
}

// create tabs and fill with init info
bool PushStorage::InitTabs() {
    if (!CreateTabs()) {
        return false;
    }
    return FillTabs();
}

// create tabs
bool PushStorage::CreateTabs() {
    for (const auto& [tab_name, create_query] : GetSqlCreateTabQueries()) {
        if (!ExecSqlQuery(create_query)) {
            return false;
        }
    }
    return true;
}

// fill tabs with information
bool PushStorage::FillTabs() {
    for (const auto& [tab_name, insert_query] : GetSqlFillTabQueries()) {
        if (!ExecSqlQuery(insert_query)) {
            return false;
        }
    }
    return true;
}

// checks if the table exists
bool PushStorage::IsTableExists(const std::string_view& tab_name) {
    sqlite3_stmt* stmt;
    const std::string sql =
        std::format("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='{}'", tab_name);

    if (sqlite3_prepare_v2(/* pointer to the database */ db_,
                           /* the sql query string */ sql.c_str(),
                           /* the sql query string is read up to the first terminating character /0 */ -1,
                           /* pointer to the compiled expression */ &stmt,
                           /* pointer to the unused part of the string */ nullptr) != SQLITE_OK) {
        last_error_ = std::move(sqlite3_errmsg(db_));
        return false;
    }

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        exists = count > 0;
    } else {
        last_error_ = std::move(sqlite3_errmsg(db_));
    }

    sqlite3_finalize(stmt);
    return exists;
}

std::unique_ptr<IPushStorage> makeStorage(const std::string& db_name) {
    return std::make_unique<PushStorage>(db_name);
}
}  // namespace push_storage
