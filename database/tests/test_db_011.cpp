#include "test_db_011.h"

#include <cassert>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "common.h"
#include "server_event_db.h"

namespace test_db {

/* =========================
 * Создание таблицы
 * ========================= */
bool CreateServerEventTable(sqlite_db::Database& db) {
    ServerEventDAO dao(db);
    return dao.InitTable();
}

/* =========================
 * Очистка таблицы
 * ========================= */
bool ClearServerEventTable(sqlite_db::Database& db) {
    const std::string sql = std::format("DELETE FROM {};", "ServerEvents");
    return db.ExecuteNonQuery(sql);
}

/* =========================
 * INSERT тест: сервер
 * ========================= */
bool TestInsertServerEvent(sqlite_db::Database& db) {
    ServerEventDAO dao(db);
    bool r = dao.AddServerEvent(ServerEventType::ServerUp, "Server started");
    if (!r) {
        std::cerr << "AddServerEvent failed: " << db.GetLastError().value_or("unknown") << std::endl;
    }
    return r;
}

/* =========================
 * INSERT тест: пользователь
 * ========================= */
bool TestInsertUserEvent(sqlite_db::Database& db) {
    ServerEventDAO dao(db);
    bool r = dao.AddUserEvent(ServerEventType::ClientAuthorized, 1, "User authorized");
    if (!r) {
        std::cerr << "AddUserEvent failed: " << db.GetLastError().value_or("unknown") << std::endl;
    }
    return r;
}

/* =========================
 * INSERT тест: push-событие
 * ========================= */
bool TestInsertPushEvent(sqlite_db::Database& db) {
    ServerEventDAO dao(db);
    bool r = dao.AddPushEvent(ServerEventType::PushSent, 42, "Push message sent");
    if (!r) {
        std::cerr << "AddPushEvent failed: " << db.GetLastError().value_or("unknown") << std::endl;
    }
    return r;
}

/* =========================
 * SELECT тест: проверка всех событий
 * ========================= */
bool TestSelectServerEvents(sqlite_db::Database& db) {
    const std::string sql =
        "SELECT event_type, owner_type, owner_id, push_id, description "
        "FROM ServerEvents ORDER BY id ASC;";

    auto result = db.ExecuteQuery(sql);
    if (!result.has_value()) {
        std::cerr << "Select failed: " << db.GetLastError().value_or("unknown") << std::endl;
        return false;
    }

    const auto& rows = result.value();
    assert(rows.size() >= 3);

    // Первое событие — сервер
    assert(std::stoi(rows[0][0]) == static_cast<int>(ServerEventType::ServerUp));
    assert(std::stoi(rows[0][1]) == static_cast<int>(EventOwnerType::Server));
    assert(rows[0][2].empty());  // owner_id = NULL
    assert(rows[0][3].empty());  // push_id = NULL
    assert(rows[0][4] == "Server started");

    // Второе событие — пользователь
    assert(std::stoi(rows[1][1]) == static_cast<int>(EventOwnerType::User));
    assert(std::stoi(rows[1][2]) == 1);
    assert(rows[1][3].empty());  // push_id = NULL
    assert(rows[1][4] == "User authorized");

    // Третье событие — push
    assert(std::stoi(rows[2][0]) == static_cast<int>(ServerEventType::PushSent));
    assert(std::stoi(rows[2][1]) == static_cast<int>(EventOwnerType::Server));
    assert(rows[2][2].empty());           // owner_id = NULL
    assert(std::stoi(rows[2][3]) == 42);  // push_id
    assert(rows[2][4] == "Push message sent");

    return true;
}

/* =========================
 * Полный тест
 * ========================= */
bool TestServerEventFull(sqlite_db::Database& db) {
    bool r1 = CreateServerEventTable(db);
    bool r2 = ClearServerEventTable(db);
    bool r3 = TestInsertServerEvent(db);
    bool r4 = TestInsertUserEvent(db);
    bool r5 = TestInsertPushEvent(db);
    bool r6 = TestSelectServerEvents(db);

    if (!r1)
        std::cerr << "CreateServerEventTable failed\n";
    if (!r2)
        std::cerr << "ClearServerEventTable failed\n";
    if (!r3)
        std::cerr << "TestInsertServerEvent failed\n";
    if (!r4)
        std::cerr << "TestInsertUserEvent failed\n";
    if (!r5)
        std::cerr << "TestInsertPushEvent failed\n";
    if (!r6)
        std::cerr << "TestSelectServerEvents failed\n";

    return r1 && r2 && r3 && r4 && r5 && r6;
}

}  // namespace test_db
