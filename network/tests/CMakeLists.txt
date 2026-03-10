include(CTest)
find_package(GTest REQUIRED)

add_executable(test_network
    src/test_network.cpp
)

target_link_libraries(test_network PRIVATE
    network_server_lib
    network_client_lib
    GTest::gtest
    GTest::gtest_main
)

add_executable(test_protocol
    src/test_protocol.cpp 
)

target_link_libraries(test_protocol PRIVATE
    protocol_lib
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME test_network COMMAND test_network)
add_test(NAME test_protocol COMMAND test_protocol)
