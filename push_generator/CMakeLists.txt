# ============================================================
# Push Generator (отдельный бинарник)
# ============================================================

cmake_minimum_required(VERSION 3.16)
project(push_generator LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# ------------------------------------------------------------
# Источники
# ------------------------------------------------------------

set(PUSH_GENERATOR_SOURCES
    src/main.cpp
    src/push_generator.cpp
    src/admin_push_generator.cpp
)

# ------------------------------------------------------------
# Исполняемый файл
# ------------------------------------------------------------

add_executable(push_generator
    ${PUSH_GENERATOR_SOURCES}
)

# ------------------------------------------------------------
# Include директории
# ------------------------------------------------------------

target_include_directories(push_generator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

        # общие модули проекта
        ${CMAKE_SOURCE_DIR}/common/push
	${CMAKE_SOURCE_DIR}/common/serialization
        ${CMAKE_SOURCE_DIR}/network/client/include
	${CMAKE_SOURCE_DIR}/network/server/include
)

# ------------------------------------------------------------
# Линковка зависимостей
# ------------------------------------------------------------


target_link_libraries(push_generator
    PRIVATE
        deps_boost          # Boost (из корневого CMake)
	    push_common
		network_client_lib
		network_server_lib	
)

# ------------------------------------------------------------
# Платформенные особенности
# ------------------------------------------------------------

if (WIN32)
    target_link_libraries(push_generator PRIVATE ws2_32)
endif()

# ------------------------------------------------------------
# Компилятор
# ------------------------------------------------------------

target_compile_features(push_generator PRIVATE cxx_std_20)

