#pragma once

#include <string>

enum class PushPriority { Normal, High, Critical };

constexpr bool IsValid(PushPriority p) noexcept {
    return p == PushPriority::Normal || p == PushPriority::High || p == PushPriority::Critical;
}

enum class PushCategory { Notification, Warning, Error };

constexpr bool IsValid(PushCategory c) noexcept {
    return c == PushCategory::Notification || c == PushCategory::Warning || c == PushCategory::Error;
}

enum class PushSource {
    // Suggested types: CEO, HR, Server, Monitoring, Calendar
    Source1,
    Source2,
    Source3
};

constexpr bool IsValid(PushSource s) noexcept {
    return s == PushSource::Source1 || s == PushSource::Source2 || s == PushSource::Source3;
}

struct Push {
    PushPriority priority{};
    PushCategory category{};
    PushSource source{};
    std::string text;
};
