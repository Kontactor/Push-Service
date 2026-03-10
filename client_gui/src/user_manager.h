#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <QString>

#include "client_lib.h"

namespace user_manager {

// Заглушка. На клиенте не доделано
struct User {
    QString username;
    client_lib::ClientType clientType;

    User() : username(""), clientType(client_lib::ClientType::Unknown) {}
    User(const QString& name, client_lib::ClientType type) : username(name), clientType(type) {}
    User(const User& other) : username(other.username), clientType(other.clientType) {}

    explicit operator bool() const {
        return !username.isEmpty();
    }
};

inline QString GetClietTypeStr(const client_lib::ClientType& clientType) {
    QString clientTypeStr;

    switch (clientType) {
        case client_lib::ClientType::Admin:
            clientTypeStr = "Администратор";
            break;

        case client_lib::ClientType::User:
            clientTypeStr = "Пользователь";
            break;

        default:
            clientTypeStr = "Неизвестно";
            break;
    }

    return clientTypeStr;
}
}  // namespace user_manager

#endif  // USER_MANAGER_H
