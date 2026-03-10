#include <iostream>

namespace ConsoleColor {
constexpr const char* RESET = "\033[0m";  // Сброс стилей
constexpr const char* RED = "\033[31m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* GREEN = "\033[32m";
// constexpr const char* BLUE = "\033[34m";
constexpr const char* CYAN = "\033[36m";
// constexpr const char* BR_RED = "\033[91m";
}  // namespace ConsoleColor

int main() {
    // Примеры использования:
    std::cout << ConsoleColor::CYAN << "[SYSTEM] Connected to server" << ConsoleColor::RESET << std::endl;
    std::cout << ConsoleColor::YELLOW << "[ERROR] Cannot connect to server" << ConsoleColor::RESET << std::endl;
    std::cout << ConsoleColor::RED << "[CRITICAL] Server down!" << ConsoleColor::RESET << std::endl;

    std::cout << ConsoleColor::GREEN << "Just green text" << ConsoleColor::RESET << std::endl;

    return 0;
}
