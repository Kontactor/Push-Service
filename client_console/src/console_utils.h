#pragma once

#include <string>

#include "push/push.h"

namespace console_ui {

namespace ConsoleColor {
inline constexpr const char* RESET = "\033[0m";
inline constexpr const char* CYAN = "\033[36m";
inline constexpr const char* GREEN = "\033[32m";
inline constexpr const char* YELLOW = "\033[33m";
inline constexpr const char* BR_RED = "\033[91m";
}  // namespace ConsoleColor

enum class ConsoleMsgType { System, Error, Hint };

struct Message {
    ConsoleMsgType type;
    std::string msg;
};

struct ConsoleMsgStyle {
    const char* label;
    const char* color;
};

// Ties the message type with a color (system, error and hint only!). Doesn't affect the color of push notifications.
// The color of push messages depends on their priority.
// When calling the 'history' command, the color of messages in the output will be the same as when they were sent.
constexpr ConsoleMsgStyle styleFor(ConsoleMsgType t);

void resetColor();

void printNewLine();

void print(const std::string& text);

void printWithColor(const std::string& text, const char* color);

const char* colorForPriority(PushPriority p);

std::string formatPush(const Push& msg);

void printMsg(ConsoleMsgType type, const std::string& msg, bool endl = true);

void hideCursor();

void showCursor();

void showLoading(int i);

void clearLine(size_t length);
}  // namespace console_ui
