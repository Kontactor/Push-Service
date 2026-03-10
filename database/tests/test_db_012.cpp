#include "test_db_012.h"

#include <cassert>
#include <format>
#include <iostream>

#include "common.h"

namespace db_util {

constexpr std::string_view kUsersTable = "users";

constexpr std::string_view kId = "id";
constexpr std::string_view kUsername = "username";
constexpr std::string_view kPasswordHash = "password_hash";
constexpr std::string_view kUserType = "user_type";
constexpr std::string_view kIsActive = "is_active";
constexpr std::string_view kCreatedAt = "created_at";
constexpr std::string_view kLastSeen = "last_seen";

constexpr std::string_view kUserPushPermissionsTable = "user_push_permissions";

constexpr std::string_view kUserId = "user_id";
constexpr std::string_view kPushTypeId = "push_type_id";
constexpr std::string_view kCanPush = "can_push";
constexpr std::string_view kCanReceive = "can_receive";

constexpr std::string_view kPushTypesTable = "push_types";

constexpr std::string_view kDescription = "description";

}  // namespace db_util

namespace test_db {

bool CreateTableIfNotExists(sqlite_db::Database& db) {
    std::cout << "=== Starting create tables ===" << std::endl;

    std::string query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "{} INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "{} TEXT NOT NULL UNIQUE, "
        "{} TEXT NOT NULL, "
        "{} TINYINT NOT NULL DEFAULT 0, "
        "{} DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "{} BOOLEAN NOT NULL DEFAULT 1, "
        "{} DATETIME DEFAULT NULL);",
        db_util::kUsersTable,
        db_util::kId,
        db_util::kUsername,
        db_util::kPasswordHash,
        db_util::kUserType,
        db_util::kCreatedAt,
        db_util::kIsActive,
        db_util::kLastSeen);

    if (!db.ExecuteNonQuery(query)) {
        std::cerr << "Create table <" << db_util::kUsersTable << "> is failed." << db.GetLastError().value()
                  << std::endl;
        return false;
    }

    query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "{} INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
        "{} TEXT NOT NULL);",
        db_util::kPushTypesTable,
        db_util::kId,
        db_util::kDescription);

    if (!db.ExecuteNonQuery(query)) {
        std::cerr << "Create table <" << db_util::kPushTypesTable << "> is failed." << db.GetLastError().value()
                  << std::endl;
        return false;
    }

    query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "{} INTEGER NOT NULL,"
        "{} TINYINT NOT NULL,"
        "{} BOOLEAN NOT NULL,"
        "{} BOOLEAN NOT NULL,"
        "PRIMARY KEY ({}, {}), "
        "FOREIGN KEY ({}) REFERENCES {}({}) ON DELETE CASCADE, "
        "FOREIGN KEY ({}) REFERENCES {}({}));",
        db_util::kUserPushPermissionsTable,
        db_util::kUserId,
        db_util::kPushTypeId,
        db_util::kCanPush,
        db_util::kCanReceive,
        // PRIMARY KEY параметры:
        db_util::kUserId,
        db_util::kPushTypeId,
        // FOREIGN KEY для user_id:
        db_util::kUserId,
        db_util::kUsersTable,
        db_util::kId,
        // FOREIGN KEY для push_type_id:
        db_util::kPushTypeId,
        db_util::kPushTypesTable,
        db_util::kId);

    if (!db.ExecuteNonQuery(query)) {
        std::cerr << "Create table <" << db_util::kUserPushPermissionsTable << "> is failed."
                  << db.GetLastError().value() << std::endl;
        return false;
    }

    std::cout << "=== Tables are created ===" << std::endl;

    return true;
}

void DeleteAllRowsBeforeTest(sqlite_db::Database& db) {
    std::cout << "Clearing all tables..." << std::endl;

    // Отключаем foreign key проверки для безопасного удаления
    db.ExecuteNonQuery("PRAGMA foreign_keys = OFF;");

    std::string query = std::format("DELETE FROM {};", db_util::kUsersTable);
    db.ExecuteNonQuery(query);

    query = std::format("DELETE FROM {};", db_util::kPushTypesTable);
    db.ExecuteNonQuery(query);

    query = std::format("DELETE FROM {};", db_util::kUserPushPermissionsTable);
    db.ExecuteNonQuery(query);

    // Включаем обратно
    db.ExecuteNonQuery("PRAGMA foreign_keys = ON;");

    std::cout << "All tables cleared" << std::endl;
}

bool TestInsertRowUsers(sqlite_db::Database& db) {
    std::cout << "=== Starting test: insert row to users ===" << std::endl;
    std::string query = std::format("INSERT INTO {} ({}, {}, {}) VALUES (?, ?, ?);",
                                    db_util::kUsersTable,
                                    db_util::kUsername,
                                    db_util::kPasswordHash,
                                    db_util::kUserType);

    auto result = db.ExecuteNonQuery(query, "user1", "user", static_cast<int>(UserType::User));

    if (!result) {
        std::cerr << "Insert row failed for username='user1':: " << db.GetLastError().value() << std::endl;
        return false;
    }

    result = db.ExecuteNonQuery(query, "admin1", "admin", static_cast<int>(UserType::Admin));

    if (!result) {
        std::cerr << "Insert row failed for username='admin1':: " << db.GetLastError().value() << std::endl;
        return false;
    }

    query = std::format("SELECT * FROM {}", db_util::kUsersTable);

    auto result_rows = db.ExecuteQuery(query);

    if (!result_rows.has_value()) {
        return false;
    }

    assert(result_rows.value().size() == 2);

    std::cout << "=== Success: insert row to users ===" << std::endl;

    return true;
}

bool TestInsertRowPushType(sqlite_db::Database& db) {
    std::cout << "=== Testing push type insertion ===" << std::endl;
    std::string query = std::format("INSERT INTO {} ({}) VALUES (?);", db_util::kPushTypesTable, db_util::kDescription);

    bool success = db.ExecuteNonQuery(query, "Type1");

    if (success) {
        std::cout << "=== Success: push type inserted ===" << std::endl;
    } else {
        std::cerr << "Failed to insert push type: " << db.GetLastError().value() << std::endl;
    }

    return success;
}

bool TestInsertRowUserPushPermissions(sqlite_db::Database& db) {
    std::string query = std::format("SELECT * FROM {} LIMIT 1", db_util::kUsersTable);

    auto result_rows = db.ExecuteQuery(query);

    if (!result_rows.has_value()) {
        std::cerr << "Failed to get user for permissions test" << std::endl;
        return false;
    }

    auto& row = result_rows.value();
    auto user_id = stoi(row[0][0]);

    query = std::format("SELECT * FROM {} LIMIT 1", db_util::kPushTypesTable);

    result_rows = db.ExecuteQuery(query);

    if (!result_rows.has_value()) {
        std::cerr << "Failed to get push type for permissions test" << std::endl;
        return false;
    }

    row = result_rows.value();
    auto type_id = stoi(row[0][0]);

    query = std::format("INSERT INTO {} ({}, {}, {}, {}) VALUES (?, ?, ?, ?);",
                        db_util::kUserPushPermissionsTable,
                        db_util::kUserId,
                        db_util::kPushTypeId,
                        db_util::kCanPush,
                        db_util::kCanReceive);

    bool success = db.ExecuteNonQuery(query, user_id, type_id, true, false);

    if (success) {
        std::cout << "=== Success: user push permissions inserted ===" << std::endl;
    } else {
        std::cerr << "Failed to insert user push permissions: " << db.GetLastError().value() << std::endl;
    }

    return success;
}

bool TestUpdateRow(sqlite_db::Database& db) {
    std::cout << "=== Testing user update ===" << std::endl;

    std::string query = std::format("UPDATE {} SET {} = ?, {} = ? WHERE {} = ?;",
                                    db_util::kUsersTable,
                                    db_util::kUsername,
                                    db_util::kUserType,
                                    db_util::kUsername);

    // Меняем username с "user1" на "user_updated" и userType с User на Admin
    auto result = db.ExecuteNonQuery(query, "user_updated", static_cast<int>(UserType::Admin), "user1");

    if (!result) {
        std::cerr << "Update failed for user1. " << db.GetLastError().value() << std::endl;
        return false;
    }

    // Проверяем, что обновление прошло успешно
    query = std::format("SELECT * FROM {} WHERE {} = ?;", db_util::kUsersTable, db_util::kUsername);

    auto result_rows = db.ExecuteQuery(query, "user_updated");

    if (!result_rows.has_value() || result_rows.value().empty()) {
        std::cerr << "No rows found after update. " << db.GetLastError().value() << std::endl;
        return false;
    }

    auto& rows = result_rows.value();

    assert(rows.size() == 1);
    assert(rows[0][1] == "user_updated");
    assert(std::stoi(rows[0][3]) == static_cast<int>(UserType::Admin));

    if (result) {
        std::cout << "=== Success: user updated ===" << std::endl;
    }

    return result;
}

bool TestDeleteRow(sqlite_db::Database& db) {
    std::cout << "=== Testing single row deletion ===" << std::endl;

    std::string insert_query = std::format("INSERT INTO {} ({}, {}, {}) VALUES (?, ?, ?);",
                                           db_util::kUsersTable,
                                           db_util::kUsername,
                                           db_util::kPasswordHash,
                                           db_util::kUserType);

    auto result = db.ExecuteNonQuery(insert_query, "temp_user", "temp_pass", 1);

    if (!result) {
        std::cerr << "Failed to insert temp user for deletion test. " << db.GetLastError().value() << std::endl;
        return false;
    }

    std::string check_query =
        std::format("SELECT COUNT(*) FROM {} WHERE {} = ?;", db_util::kUsersTable, db_util::kUsername);

    auto count_result = db.ExecuteQuery(check_query, "temp_user");

    if (!count_result.has_value() || count_result.value().empty()) {
        std::cerr << "Failed to verify inserted temp user. " << db.GetLastError().value() << std::endl;
        return false;
    }

    std::string delete_query = std::format("DELETE FROM {} WHERE {} = ?;", db_util::kUsersTable, db_util::kUsername);

    result = db.ExecuteNonQuery(delete_query, "temp_user");

    if (!result) {
        std::cerr << "Delete failed\n";
        return false;
    }

    count_result = db.ExecuteQuery(check_query, "temp_user");

    if (!count_result.has_value() || count_result.value().empty()) {
        std::cerr << "Failed to verify deletion\n";
        return false;
    }

    int count_after = std::stoi(count_result.value()[0][0]);
    if (count_after != 0) {
        std::cerr << "Deletion verification failed: user still exists (count = " << count_after << ")\n";
        return false;
    }

    if (result && count_after == 0) {
        std::cout << "=== Success: single row deletion ===" << std::endl;
    }

    return result && (count_after == 0);
}

bool TestDeleteCascade(sqlite_db::Database& db) {
    std::cout << "=== Starting cascade delete test ===" << std::endl;

    // 1. Находим пользователя "user_updated"
    std::string query =
        std::format("SELECT {} FROM {} WHERE {} = ? LIMIT 1", db_util::kId, db_util::kUsersTable, db_util::kUsername);

    auto result_rows = db.ExecuteQuery(query, "user_updated");

    if (!result_rows.has_value() || result_rows.value().empty()) {
        std::cerr << "Failed cascade delete test. User 'user_updated' not found. " << db.GetLastError().value()
                  << std::endl;
        return false;
    }

    auto& row = result_rows.value();
    int user_id = std::stoi(row[0][0]);
    std::cout << "Found user 'user_updated' with ID: " << user_id << std::endl;

    // 2. Проверяем, что у пользователя есть разрешения
    query = std::format("SELECT COUNT(*) FROM {} WHERE {} = ?;", db_util::kUserPushPermissionsTable, db_util::kUserId);

    result_rows = db.ExecuteQuery(query, user_id);

    if (!result_rows.has_value() || result_rows.value().empty()) {
        std::cerr << "Failed cascade delete test: Cannot get permissions count. " << db.GetLastError().value()
                  << std::endl;
        return false;
    }

    int permissions_count = std::stoi(result_rows.value()[0][0]);
    std::cout << "User " << user_id << " has " << permissions_count << " permissions before cascade delete"
              << std::endl;

    if (permissions_count == 0) {
        std::cerr << "Cannot test cascade - user has no permissions" << std::endl;
        return false;
    }

    // 3. Удаляем пользователя
    query = std::format("DELETE FROM {} WHERE {} = ?;", db_util::kUsersTable, db_util::kId);

    if (!db.ExecuteNonQuery(query, user_id)) {
        std::cerr << "Failed cascade delete. " << db.GetLastError().value() << std::endl;
        return false;
    }

    std::cout << "User " << user_id << " deleted" << std::endl;

    // 4. Проверяем, что пользователь удалён
    query = std::format("SELECT COUNT(*) FROM {} WHERE {} = ?;", db_util::kUsersTable, db_util::kId);

    result_rows = db.ExecuteQuery(query, user_id);
    if (!result_rows.has_value() || result_rows.value().empty()) {
        std::cerr << "Failed to verify user deletion. " << db.GetLastError().value() << std::endl;
        return false;
    }

    int user_count_after = std::stoi(result_rows.value()[0][0]);
    if (user_count_after != 0) {
        std::cerr << "Failed cascade delete. User still exists in users (count = " << user_count_after << ")"
                  << std::endl;
        return false;
    }

    // 5. Проверяем, что разрешения каскадно удалены
    query = std::format("SELECT COUNT(*) FROM {} WHERE {} = ?;", db_util::kUserPushPermissionsTable, db_util::kUserId);

    result_rows = db.ExecuteQuery(query, user_id);
    if (!result_rows.has_value() || result_rows.value().empty()) {
        std::cerr << "Failed to verify permissions deletion. " << db.GetLastError().value() << std::endl;
        return false;
    }

    int permissions_count_after = std::stoi(result_rows.value()[0][0]);
    if (permissions_count_after != 0) {
        std::cerr << "Failed cascade delete. User_id still has " << permissions_count_after
                  << " permissions in user_push_permissions" << std::endl;
        return false;
    }

    std::cout << "=== Cascade delete test passed successfully! ===" << std::endl;

    return true;
}

bool TestFull(sqlite_db::Database& db) {
    bool ret = true;

    std::cout << "STARTING DATABASE TESTS" << std::endl;

    if (!CreateTableIfNotExists(db)) {
        std::cerr << "CreateTable failed" << std::endl;
        ret = false;
    }

    DeleteAllRowsBeforeTest(db);

    if (!TestInsertRowUsers(db)) {
        std::cerr << "TestInsertRowUsers failed" << std::endl;
        ret = false;
    }

    if (!TestInsertRowPushType(db)) {
        std::cerr << "TestInsertRowPushType failed" << std::endl;
        ret = false;
    }

    if (!TestInsertRowUserPushPermissions(db)) {
        std::cerr << "TestInsertRowUserPushPermissions failed" << std::endl;
        ret = false;
    }

    if (!TestUpdateRow(db)) {
        std::cerr << "TestUpdateRow failed" << std::endl;
        ret = false;
    }

    if (!TestDeleteRow(db)) {
        std::cerr << "TestDeleteRow failed" << std::endl;
        ret = false;
    }

    if (!TestDeleteCascade(db)) {
        std::cerr << "TestDeleteRow failed" << std::endl;
        ret = false;
    }

    if (ret) {
        std::cout << "ALL TESTS PASSED SUCCESSFULLY!" << std::endl;
    } else {
        std::cout << "SOME TESTS FAILED" << std::endl;
    }

    return ret;
}
}  // namespace test_db
