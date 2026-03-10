add_executable(server_demo src/server_demo.cpp)
target_link_libraries(server_demo PRIVATE network_server_lib)

add_executable(client_demo src/client_demo.cpp)
target_link_libraries(client_demo PRIVATE network_client_lib)
