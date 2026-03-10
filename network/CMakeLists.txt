add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(protocol)
add_subdirectory(tests)

option(NETWORK_BUILD_EXAMPLES "Build network examples" OFF)
if(NETWORK_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
