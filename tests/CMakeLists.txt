# ============================================================
# CMakeLists для GoogleTest-based tests 
# ============================================================

# Создаём исполняемый файл с тестами
add_executable(push_cpp_tests
    test_dummy.cpp        # тест-заглушка
)

# Линкуем зависимости
target_link_libraries(push_cpp_tests
    PRIVATE
        client_lib          # общая библиотека проекта
        deps_gtest          # GoogleTest (из корневого CMake)
)

# Регистрирует тест в CMake/CTest. Делает его видимым для: комманды ctest
add_test(
    NAME PushCppTests
    COMMAND push_cpp_tests
)



