#include "test_db_013.h"

#include <iostream>
#include <sqlite3.h>

#include "common.h"
#include "server_event_db.h"

namespace test_db {

static bool Exec(sqlite3* db, const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "SQL error: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool TestTriggerLastSeen(sqlite_db::Database& db) {
    std::cout << "Running DB-013 (trigger last_seen)..." << std::endl;

    sqlite3* raw = db.Handle();

    // 1. Создаём таблицу users
    if (!Exec(raw,
              "DROP TABLE IF EXISTS users;"
              "CREATE TABLE users ("
              "id INTEGER PRIMARY KEY,"
              "username TEXT,"
              "last_seen TEXT"
              ");")) {
        return false;
    }

    // 2. Добавляем тестового пользователя
    if (!Exec(raw,
              "INSERT INTO users (id, username, last_seen) "
              "VALUES (1, 'test_user', NULL);")) {
        return false;
    }

    // 3. Инициализируем ServerEventDAO (создаст таблицу + триггер)
    ServerEventDAO dao(db);
    if (!dao.InitTable()) {
        std::cerr << "InitTable failed" << std::endl;
        return false;
    }

    // 4. Добавляем событие ClientConnected
    if (!dao.AddUserEvent(ServerEventType::ClientConnected, 1, "User connected")) {
        std::cerr << "AddUserEvent failed" << std::endl;
        return false;
    }

    // 5. Проверяем last_seen
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT last_seen FROM users WHERE id = 1;";

    if (sqlite3_prepare_v2(raw, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SELECT prepare failed" << std::endl;
        return false;
    }

    std::string last_seen;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        last_seen = text ? reinterpret_cast<const char*>(text) : "";
    } else {
        std::cerr << "SELECT step failed" << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (last_seen.empty()) {
        std::cerr << "Trigger did NOT update last_seen!" << std::endl;
        return false;
    }

    std::cout << "Trigger updated last_seen = " << last_seen << std::endl;

    // 6. Добавляем событие ClientReconnected
    if (!dao.AddUserEvent(ServerEventType::ClientReconnected, 1, "User reconnected")) {
        std::cerr << "AddUserEvent failed" << std::endl;
        return false;
    }

    // 7. Проверяем, что last_seen обновилось снова
    if (sqlite3_prepare_v2(raw, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SELECT prepare failed" << std::endl;
        return false;
    }

    std::string last_seen2;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        last_seen2 = text ? reinterpret_cast<const char*>(text) : "";
    } else {
        std::cerr << "SELECT step failed" << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (last_seen2 == last_seen) {
        std::cerr << "Trigger did NOT update last_seen on reconnect!" << std::endl;
        return false;
    }

    std::cout << "Trigger updated last_seen again = " << last_seen2 << std::endl;
    std::cout << "DB-013 PASSED!" << std::endl;

    return true;
}

}  // namespace test_db
