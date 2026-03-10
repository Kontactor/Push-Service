#include "test.h"

#include <cassert>
#include <cstdio>
#include <format>
#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <string>
#include <vector>

#include "push_storage.h"

using namespace push_storage;
using namespace std::literals;

IPush GetRandomPush() {
    const std::vector<std::string> payloads = {"'Important': \"Check\" your 'security' settings"s,
                                               "'Reminder': It's \"time\" to complete the task"s,
                                               "Your 'order' has been shipped"s,
                                               "A gift awaits: \"activate\" the 'coupon'"s,
                                               "Be 'proud' of yourself — you tried"s,
                                               "Inspiration of the \"day\": a quote for you"s,
                                               "Your 'action' is waiting for confirmation!"s,
                                               "New message — open now"s,
                                               "You missed the fun — come back!"s,
                                               "The update is available — install it now"s,
                                               "Thank you for your trust! We appreciate you"s,
                                               "You have a new notification, take a look"s,
                                               "30% discount only today — don't miss it!"s,
                                               "Product in stock: order now"s,
                                               "We recommend: products according to your 'interests'"s,
                                               "New subscriber — meet me!"s,
                                               "You were mentioned in a 'comment'"s,
                                               "The payment was successful"s,
                                               "The card limit has been 'updated'"s,
                                               "Cashback is waiting: spend it now"s};
    return IPush{static_cast<PushType>(rand() % 2),
                 static_cast<PushPriority>(rand() % 3),
                 payloads[rand() % payloads.size()]};
}

void PrintError(std::unique_ptr<push_storage::IPushStorage>& push_storage) {
    if (const auto& error = push_storage->GetLastError(); error) {
        std::cerr << error.value();
        return;
    }
};

namespace test_db {

void CreateAndFillDB(const std::string& db_name) {
    auto push_storage = makeStorage(db_name);

    if (!push_storage->OpenDatabase()) {
        PrintError(push_storage);
        return;
    }

    const int records_count = 10;

    for (int i = 0; i < records_count; ++i) {
        if (!push_storage->InsertPush(rand() % 2, GetRandomPush())) {
            PrintError(push_storage);
            return;
        }
    }
}

void PrintDBTab(const std::string& db_name) {
    const std::string PUSH_EVENTS_TAB = "PushEvents"s;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open(db_name.c_str(), &db)) {
        return;
    }

    std::string sql_select = "SELECT * FROM "s + PUSH_EVENTS_TAB;
    if (sqlite3_prepare_v2(db, sql_select.c_str(), -1, &stmt, nullptr)) {
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const int id = sqlite3_column_int(stmt, 0);
        const int type_id = sqlite3_column_int(stmt, 1);
        const int priority_id = sqlite3_column_int(stmt, 2);
        const std::string payload = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const int sender_id = sqlite3_column_int(stmt, 4);
        const std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        std::string res_row = std::format("{} {} {} {} {} {}", id, type_id, priority_id, payload, sender_id, timestamp);
        std::cout << res_row << std::endl;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

bool TestSelectElementsType1() {
    std::cout << __FUNCTION__ << "..."s;
    std::ostringstream expected_res_out;

    const std::string db_name = "test.db"s;
    const std::string tab_name = "PushEvents"s;
    {
        auto push_storage = makeStorage(db_name);
        if (!push_storage->OpenDatabase()) {
            std::cout << "fail\n"s;
            PrintError(push_storage);
            return false;
        }

        int records_count = 10;
        for (int i = 0; i < records_count; ++i) {
            IPush push = GetRandomPush();
            int sender_id = rand() % 2;
            if (push.type == PushType::Type1) {
                expected_res_out << static_cast<int>(push.type) << ' ' << sender_id << ' ' << push.payload << '\n';
            }
            if (!push_storage->InsertPush(sender_id, push)) {
                std::cout << "fail\n"s;
                PrintError(push_storage);
                return false;
            }
        }
    }

    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open(db_name.c_str(), &db)) {
        std::cout << "Failed to open database " << db_name << std::endl;
        return false;
    }

    std::string sql_select = "SELECT type_id, sender_id, payload FROM "s + tab_name + " WHERE type_id = 0"s;
    if (sqlite3_prepare_v2(/* pointer to the database */ db,
                           /* the sql query string */ sql_select.c_str(),
                           /* the sql query string is read up to the first terminating character /0 */ -1,
                           /* pointer to the compiled expression */ &stmt,
                           /* pointer to the unused part of the string */ nullptr) != SQLITE_OK) {
        std::cout << "Fail to exec sql query: "s << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    std::ostringstream res;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int type = sqlite3_column_int(stmt, 0);
        const unsigned char* sender = sqlite3_column_text(stmt, 1);
        const unsigned char* payload = sqlite3_column_text(stmt, 2);
        res << type << ' ' << sender << ' ' << payload << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    // delete the file with db so that the test does not fail when rerunning again
    std::remove(db_name.c_str());

    assert(expected_res_out.str() == res.str());

    std::cout << " OK"s;
    return true;
}

}  // namespace test_db
