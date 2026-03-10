#pragma once

#include <string>

enum PushType { Type1, Type2 };

enum PushPriority { Notification, Warning, Alert };

class IPush {
public:
    PushType type;
    PushPriority priority;
    std::string payload;
};

/* =========================
 * Тип пользователя
 * ========================= */
enum class UserType { User, Admin };

/* =========================
 * Типы серверных событий
 * ========================= */
enum class ServerEventType {
    ClientConnected,
    PushSent,
    ClientDisconnected,
    ClientReconnected,
    ClientAuthorized,
    PushAccepted,
    ServerUp,
    ServerDown
};

/* =========================
 * Владелец события
 * ========================= */
enum class EventOwnerType { Server = 0, User = 1 };
