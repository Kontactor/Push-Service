#pragma once

#include "sqlite_db.h"

namespace test_db {

bool CreateServerEventTable(sqlite_db::Database& db);
bool ClearServerEventTable(sqlite_db::Database& db);
bool TestInsertServerEvent(sqlite_db::Database& db);
bool TestSelectServerEvents(sqlite_db::Database& db);

bool TestServerEventFull(sqlite_db::Database& db);

}  // namespace test_db
