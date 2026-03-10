#include <filesystem>
#include <iostream>
#include <string>

#include "sqlite_db.h"

// Тесты
#include "test.h"
#include "test_db_011.h"
#include "test_db_012.h"
#include "test_db_013.h"

using namespace std::literals;

int main() {
    auto exe_path = std::filesystem::current_path();
    std::string db_path = (exe_path / "push-cpp-server.db").string();

    // Основная БД для тестов DB-011, DB-012, DB-013
    sqlite_db::Database db(db_path);

    // ===== DB-003 =====
    std::cout << "TEST DB-003 BEGIN" << std::endl;

    const std::string db_name = "push_events.db"s;
    test_db::CreateAndFillDB(db_name);
    test_db::PrintDBTab(db_name);
    test_db::TestSelectElementsType1();

    std::cout << "TEST DB-003 END" << std::endl;
    std::cout << "---------------" << std::endl;

    // ===== DB-011 =====
    std::cout << "TEST DB-011 BEGIN" << std::endl;

    if (test_db::TestServerEventFull(db)) {
        std::cout << "All Tests DB-011 are passed!" << std::endl;
    } else {
        std::cout << "Tests DB-011 are not passed!" << std::endl;
    }

    std::cout << "TEST DB-011 END" << std::endl;
    std::cout << "------------------" << std::endl;

    // ===== DB-012 =====
    std::cout << "TEST DB-012 BEGIN" << std::endl;

    test_db::TestFull(db);

    std::cout << "TEST DB-012 END" << std::endl;
    std::cout << "------------------" << std::endl;

    // ===== DB-013 (trigger last_seen) =====
    std::cout << "TEST DB-013 BEGIN" << std::endl;

    if (test_db::TestTriggerLastSeen(db)) {
        std::cout << "DB-013 passed!" << std::endl;
    } else {
        std::cout << "DB-013 failed!" << std::endl;
    }

    std::cout << "TEST DB-013 END" << std::endl;
    std::cout << "------------------" << std::endl;

    return 0;
}
