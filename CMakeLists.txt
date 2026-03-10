# ============================================================
# Корневой (главный) CMakeLists
# ============================================================

# ============================================================
# Минимальная версия CMake
# ============================================================
cmake_minimum_required(VERSION 3.16)


# ============================================================
# Проект
# ============================================================
project(
    Push-Cpp
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


# ============================================================
# Стандарт C++
# ============================================================

# Устанавливаем стандарт С++ 20 (нужен для потоков)
set(CMAKE_CXX_STANDARD 20)

# Если компилятор не поддерживает C++20 сборка должна упасть
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Запрещает нестандартные расширения компилятора (код должен собираться на разных компиляторах одинаково)
set(CMAKE_CXX_EXTENSIONS OFF)


# ============================================================
# Опции проекта - переключатель конфигурации проекта
# ============================================================

# В проекте есть GUI-клиент (Qt).  По умолчанию — НЕ собирать
option(BUILD_GUI "Build Qt GUI client" OFF)

# В проекте будут тесты (GoogleTest). По умолчанию —  собирать
option(BUILD_TESTS     "Build tests"         ON)

# По умолчанию выключено Qt тестирование, чтобы не тянуть Qt тестирование без нужды
option(BUILD_QT_TESTS  "Build Qt GUI tests"  OFF)


# ============================================================
# Автоматическая логика: если GUI и тесты включены, Qt Tests активны
# ============================================================

if (BUILD_GUI AND BUILD_TESTS)

# принудительно включаем BUILD_QT_TESTS=ON, даже если пользователь не задавал этот флаг
# CACHE BOOL ... FORCE означает:сохраняем значение в CMakeCache, чтобы оно было видно при следующей конфигурации
    set(BUILD_QT_TESTS ON CACHE BOOL "Build Qt GUI tests" FORCE)
endif()


# ============================================================
# Определение платформы
# ============================================================
if (WIN32)
    message(STATUS "Platform: Windows")
    add_compile_definitions(PLATFORM_WINDOWS)
elseif (UNIX AND NOT APPLE)
    message(STATUS "Platform: Linux")
    add_compile_definitions(PLATFORM_LINUX)
elseif (APPLE)
    message(STATUS "Platform: macOS")
    add_compile_definitions(PLATFORM_MACOS)
endif()


# ============================================================
# Определение компилятора
# ============================================================
if (MSVC)
    message(STATUS "Compiler: MSVC")
    # Включаем флаги компиляторов для их корректной работы
    add_compile_options(/W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Compiler: GCC")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Compiler: Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# ============================================================
# Зависимости (Conan 2)
# ============================================================

# Conan генерирует CMake config-файлы:
#   BoostConfig.cmake
#   GTestConfig.cmake
#   SQLite3Config.cmake
#
# Поэтому мы используем ОБЫЧНЫЙ find_package()


# -----------------------------
# Boost
# -----------------------------

find_package(Boost REQUIRED COMPONENTS system filesystem thread) # Ищем установленную библиотеку Boost на системе.

message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")

# Создаёт интерфейсную библиотеку CMake: INTERFACE означает: сама библиотека не компилируется
# используется только как контейнер зависимостей для линковки и include-директории.
# Теперь любой target(например server), который ссылается на deps_boost, автоматически получит:
# include директории Boost и линковку с нужными Boost библиотеками.
add_library(deps_boost INTERFACE)
target_link_libraries(deps_boost INTERFACE
    #Boost::system
    Boost::headers
    Boost::filesystem
    Boost::thread
)


# -----------------------------
# SQLite3
# -----------------------------
find_package(SQLite3 REQUIRED) # Ищем SQLite3 установленную на системе.
# paths
set(SERVER_SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/server/src")
set(SERVER_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/server/tests")

# server
add_subdirectory(server)

add_library(deps_sqlite INTERFACE)
target_link_libraries(deps_sqlite INTERFACE
    SQLite::SQLite3
)


# -----------------------------
# Отвечает за подключение библиотеки GoogleTest
# GTest (только если тесты) (аналогично как с Boost)
# -----------------------------
if (BUILD_TESTS)
    find_package(GTest REQUIRED)
    message(STATUS "GTest found")

    add_library(deps_gtest INTERFACE)
    target_link_libraries(deps_gtest INTERFACE
        GTest::gtest
        GTest::gtest_main
    )
endif()


# -----------------------------
# Qt (GUI) (аналогично как с Boost)
# -----------------------------
if (BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    # Qt Test подключаем только если включены Qt Tests
    if (BUILD_QT_TESTS)
        find_package(Qt6 REQUIRED COMPONENTS Test)
    endif()
endif()

# ============================================================
# Тестирование (включаем CTest ДО всех подпроектов)
# ============================================================
if (BUILD_TESTS)
    enable_testing()
endif()

# ============================================================
# Подпроекты
# ============================================================

add_subdirectory(client_console)

if (BUILD_GUI)
    add_subdirectory(client_gui)
endif()

add_subdirectory(client_lib)

# database содержит add_test(), поэтому должен идти ПОСЛЕ enable_testing()
add_subdirectory(database)

add_subdirectory(network)

add_subdirectory(common)

add_subdirectory(push_generator)

# ============================================================
# Тестирование (включаем CTest ДО всех подпроектов)
# ============================================================
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ============================================================
# Сводка конфигурации
# ============================================================
message(STATUS "---------------------------------------")
message(STATUS "Project:        ${PROJECT_NAME}")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "C++ standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "Build GUI:      ${BUILD_GUI}")
message(STATUS "Build tests:    ${BUILD_TESTS}")
message(STATUS "Build Qt tests: ${BUILD_QT_TESTS}")

message(STATUS "---------------------------------------")









