#pragma once
#include "sqlite_db.h"

namespace test_db {
bool CreateTableIfNotExists(sqlite_db::Database& db);
void DeleteAllRowsBeforeTest(sqlite_db::Database& db);
bool TestInsertRowUsers(sqlite_db::Database& db);
bool TestInsertRowPushType(sqlite_db::Database& db);
bool TestInsertRowUserPushPermissions(sqlite_db::Database& db);
bool TestUpdateRow(sqlite_db::Database& db);
bool TestDeleteRow(sqlite_db::Database& db);
bool TestDeleteCascade(sqlite_db::Database& db);
bool TestFull(sqlite_db::Database& db);
}  // namespace test_db
