#pragma once
#include <string>

#include "sqlite_db.h"

namespace test_db {

bool TestTriggerLastSeen(sqlite_db::Database& db);

}  // namespace test_db
