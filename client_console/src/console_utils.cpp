#include "console_utils.h"

#include <iostream>
#include <sstream>
#include <thread>

namespace console_ui {

inline std::ostream& operator<<(std::ostream& os, PushCategory category) {
    switch (category) {
        case PushCategory::Notification:
            return os << "Notification";
        case PushCategory::Warning:
            return os << "Warning";
        case PushCategory::Error:
            return os << "Error";
        default:
            return os << "UNK";
    }
    return os;
}

constexpr ConsoleMsgStyle styleFor(ConsoleMsgType t) {
    switch (t) {
        case ConsoleMsgType::System:
            return {"[SYSTEM]", ConsoleColor::CYAN};
        case ConsoleMsgType::Error:
            return {"[ERROR]", ConsoleColor::YELLOW};
        case ConsoleMsgType::Hint:
            return {"[HINT]", ConsoleColor::RESET};
    }
    return {"", ConsoleColor::RESET};
}

void resetColor() {
    std::cout << ConsoleColor::RESET;
}

void printNewLine() {
    std::cout << std::endl;
}

void print(const std::string& text) {
    resetColor();
    std::cout << text << std::flush;
}

void printWithColor(const std::string& text, const char* color) {
    std::cout << "\r" << color << text;
}

const char* colorForPriority(PushPriority priority) {
    switch (priority) {
        case PushPriority::Normal:
            return ConsoleColor::GREEN;
        case PushPriority::High:
            return ConsoleColor::YELLOW;
        case PushPriority::Critical:
            return ConsoleColor::BR_RED;
        default:
            return ConsoleColor::CYAN;
    }
    return ConsoleColor::RESET;
}

std::string formatPush(const Push& push) {
    std::ostringstream oss;
    oss << "[PUSH][" << push.category << "] " << push.text;
    return oss.str();
}

void printMsg(ConsoleMsgType type, const std::string& msg, bool endl) {
    const auto style = styleFor(type);
    std::string line = std::string(style.label) + " " + msg;

    printWithColor(line, style.color);

    if (endl) {
        printNewLine();
    }
}

void hideCursor() {
    std::cout << "\033[?25l";
}

void showCursor() {
    std::cout << "\033[?25h";
}

void showLoading(int i) {
    switch (i % 4) {
        case 0:
            std::cout << '|';
            break;
        case 1:
            std::cout << '/';
            break;
        case 2:
            std::cout << '-';
            break;
        case 3:
            std::cout << '\\';
            break;
    }
    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << '\b';
}

void clearLine(size_t length) {
    std::cout << '\r' << std::string(length, ' ') << '\r';
    std::cout.flush();
}

}  // namespace console_ui
