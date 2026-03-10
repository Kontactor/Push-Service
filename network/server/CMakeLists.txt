add_library(network_server_lib
    src/ClientRegistry.cpp
    src/TcpServer.cpp
    src/TcpSession.cpp
)

target_include_directories(network_server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(network_server_lib PUBLIC
    Boost::boost
    ${Boost_SYSTEM_LIBRARY}
)
