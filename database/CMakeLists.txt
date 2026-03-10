# =============================
# Database library
# =============================
add_library(database
    src/push_storage.cpp
    src/server_event_db.cpp
)

target_include_directories(database
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# =============================
# SQLite3
# =============================
find_package(SQLite3 REQUIRED)

target_link_libraries(database
    PUBLIC
        SQLite::SQLite3
)

# =============================
# Database tests
# =============================
add_executable(database_tests
    tests/test.cpp
    tests/test.h
    tests/test_db_011.cpp
    tests/test_db_011.h
    tests/test_db_012.cpp
    tests/test_db_012.h
	tests/test_db_013.cpp
    tests/test_db_013.h	
    src/sqlite_db.cpp
    src/main.cpp
)

target_include_directories(database_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_link_libraries(database_tests
    PRIVATE
        database
        SQLite::SQLite3
)


# =============================
# CTest
# =============================
enable_testing()

add_test(
    NAME database_tests
    COMMAND database_tests
)
