add_library(network_client_lib
    src/TcpClient.cpp
)

target_include_directories(network_client_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(network_client_lib PUBLIC
    Boost::boost
    ${Boost_SYSTEM_LIBRARY}
)
