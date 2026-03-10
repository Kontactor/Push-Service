#pragma once

#include <string>

namespace test_db {
void CreateAndFillDB(const std::string& db_name);
void PrintDBTab(const std::string& db_name);

bool TestSelectElementsType1();
}  // namespace test_db
