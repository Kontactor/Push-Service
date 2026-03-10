add_library(server_lib
    src/server.cpp
    src/dispatcher.cpp
)

target_include_directories(server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(tests)

option(SERVER_BUILD_EXAMPLES "Build server examples" OFF)
if(SERVER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
