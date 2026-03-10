#include "server_event_db.h"

#include <chrono>
#include <format>
#include <iomanip>
#include <sstream>

/* =========================
 * Table / column names
 * ========================= */
static constexpr const char* kTableName = "ServerEvents";

static constexpr const char* COL_ID = "id";
static constexpr const char* COL_EVENT_TYPE = "event_type";
static constexpr const char* COL_PUSH_ID = "push_id";
static constexpr const char* COL_OWNER_TYPE = "owner_type";
static constexpr const char* COL_OWNER_ID = "owner_id";
static constexpr const char* COL_DESCRIPTION = "description";
static constexpr const char* COL_TIMESTAMP = "timestamp";

/* =========================
 * Helpers
 * ========================= */
namespace {

std::string GetCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

/*
 * Creates SQLite trigger that updates users.last_seen
 * when user connects or reconnects
 */
bool CreateLastSeenTrigger(sqlite_db::Database& db) {
    const std::string sql = R"(
        CREATE TRIGGER IF NOT EXISTS trg_update_user_last_seen
        AFTER INSERT ON ServerEvents
        FOR EACH ROW
        WHEN
            NEW.owner_type = 1
            AND NEW.event_type IN (0, 3)
        BEGIN
            UPDATE users
            SET last_seen = NEW.timestamp
            WHERE id = NEW.owner_id;
        END;
    )";

    return db.ExecuteNonQuery(sql);
}

}  // namespace

/* =========================
 * ServerEventDAO
 * ========================= */
ServerEventDAO::ServerEventDAO(sqlite_db::Database& db) : db_(db) {}

/* =========================
 * Init table + trigger
 * ========================= */
bool ServerEventDAO::InitTable() {
    db_.ExecuteNonQuery("DROP TABLE IF EXISTS ServerEvents;");

    const std::string sql = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "{} INTEGER PRIMARY KEY AUTOINCREMENT, "
        "{} INTEGER NOT NULL, "
        "{} INTEGER, "
        "{} INTEGER NOT NULL, "
        "{} INTEGER, "
        "{} TEXT, "
        "{} TEXT NOT NULL"
        ");",
        kTableName,
        COL_ID,
        COL_EVENT_TYPE,
        COL_PUSH_ID,
        COL_OWNER_TYPE,
        COL_OWNER_ID,
        COL_DESCRIPTION,
        COL_TIMESTAMP);

    if (!db_.ExecuteNonQuery(sql))
        return false;

    // Create trigger for automatic users.last_seen update
    return CreateLastSeenTrigger(db_);
}

/* =========================
 * Server event
 * ========================= */
bool ServerEventDAO::AddServerEvent(ServerEventType type, const std::string& description) {
    const std::string sql = std::format(
        "INSERT INTO {} ({}, {}, {}, {}, {}, {}) "
        "VALUES (?, NULL, ?, NULL, ?, ?);",
        kTableName,
        COL_EVENT_TYPE,
        COL_PUSH_ID,
        COL_OWNER_TYPE,
        COL_OWNER_ID,
        COL_DESCRIPTION,
        COL_TIMESTAMP);

    return db_.ExecuteNonQuery(sql,
                               static_cast<int>(type),
                               static_cast<int>(EventOwnerType::Server),
                               description,
                               GetCurrentTime());
}

/* =========================
 * User event
 * ========================= */
bool ServerEventDAO::AddUserEvent(ServerEventType type, int user_id, const std::string& description) {
    const std::string sql = std::format(
        "INSERT INTO {} ({}, {}, {}, {}, {}, {}) "
        "VALUES (?, NULL, ?, ?, ?, ?);",
        kTableName,
        COL_EVENT_TYPE,
        COL_PUSH_ID,
        COL_OWNER_TYPE,
        COL_OWNER_ID,
        COL_DESCRIPTION,
        COL_TIMESTAMP);

    return db_.ExecuteNonQuery(sql,
                               static_cast<int>(type),
                               static_cast<int>(EventOwnerType::User),
                               user_id,
                               description,
                               GetCurrentTime());
}

/* =========================
 * Push-related event
 * ========================= */
bool ServerEventDAO::AddPushEvent(ServerEventType type, int push_id, const std::string& description) {
    const std::string sql = std::format(
        "INSERT INTO {} ({}, {}, {}, {}, {}, {}) "
        "VALUES (?, ?, ?, NULL, ?, ?);",
        kTableName,
        COL_EVENT_TYPE,
        COL_PUSH_ID,
        COL_OWNER_TYPE,
        COL_OWNER_ID,
        COL_DESCRIPTION,
        COL_TIMESTAMP);

    return db_.ExecuteNonQuery(sql,
                               static_cast<int>(type),
                               push_id,
                               static_cast<int>(EventOwnerType::Server),
                               description,
                               GetCurrentTime());
}
