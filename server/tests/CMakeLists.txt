include(CTest)
find_package(GTest REQUIRED)

add_executable(test_dispatcher src/test_dispatcher.cpp
    src/tests_utils.cpp
)

target_include_directories(test_dispatcher PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_dispatcher PRIVATE
    server_lib
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME test_dispatcher COMMAND test_dispatcher)