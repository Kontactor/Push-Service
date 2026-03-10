# ============================================================
# CMakeLists.txt для client_lib
# ============================================================

# Создаём статическую или объектную библиотеку
add_library(client_lib STATIC
    src/client_lib.cpp
    src/client_lib.h
)

# Исходники библиотеки
target_sources(client_lib
    PRIVATE
        src/client_lib.cpp
        src/client_lib.h
)

# Заголовки доступны всем, кто линкуется к client_lib
# Например: в client_console/src/main.cpp теперь можно писать этот include (#include "client_lib.h")

target_include_directories(client_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Зависимости библиотеки
# PUBLIC — потому что заголовки client_lib.h используют Boost
target_link_libraries(client_lib
    PUBLIC
        network_client_lib
        protocol_lib
)

# ============================================================
# Тесты client_lib
# ============================================================

# Создаём тестовый исполняемый файл
add_executable(client_lib_test
    tests/test_client_lib.cpp
)

# Линкуем тест с библиотекой
target_link_libraries(client_lib_test
    PRIVATE
        client_lib
        deps_gtest        # если ты используешь GoogleTest
)

# Регистрируем тест для CTest
add_test(NAME client_lib_test COMMAND client_lib_test)

# ============================================================
# Демо client_lib
# ============================================================

option(CLIENT_LIB_BUILD_EXAMPLES "Build client_lib examples" OFF)
if(CLIENT_LIB_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
