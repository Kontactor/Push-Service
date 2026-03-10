# ============================================================
# client_console/CMakeLists.txt
# ============================================================
# ------------------------------------------------------------
# Опции подпроекта
# ------------------------------------------------------------

# ------------------------------------------------------------
# Исходные файлы client_console
# ------------------------------------------------------------

set(CLIENT_CONSOLE_SOURCES
    src/client_console.cpp
    src/client_console_ui.cpp
    src/client_event_handler.cpp
    src/console_utils.cpp
)

set(CLIENT_CONSOLE_HEADERS
    src/client_console_ui.h
    src/client_event_handler.h
    src/console_utils.h
)

# ------------------------------------------------------------
# Исполняемый файл
# ------------------------------------------------------------

add_executable(client_console
    ${CLIENT_CONSOLE_SOURCES}
    ${CLIENT_CONSOLE_HEADERS}
)

# ------------------------------------------------------------
# Include-директории
# ------------------------------------------------------------

target_include_directories(client_console
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# ------------------------------------------------------------
# Выбор fake / real клиента
# ------------------------------------------------------------
target_link_libraries(client_console
    PRIVATE client_lib
)
