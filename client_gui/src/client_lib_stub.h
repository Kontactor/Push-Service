// HEADER для создание заглушек клиентской библиотеки
#ifndef CLIENT_LIB_STUB_H
#define CLIENT_LIB_STUB_H

#include <string>

#include "client_lib.h"

namespace client_lib {
class PushMessageHandlerStub : public IPushMessageHandler {
public:
    PushMessageHandlerStub() = default;
    ~PushMessageHandlerStub() override = default;

    void onPushMessage(const std::string& message) override {
        // Заглушка - ничего не делаем
    }

    void onConnectionStatusChanged(bool connected) override {
        // Заглушка - ничего не делаем
    }

    void onError(const std::string& error) override {
        // Заглушка - ничего не делаем
    }
};

// TODO
inline void connect(const std::string& server_host, int port) {
    // Заглушка - ничего не делаем
}

// TODO
inline void disconnect() {
    // Заглушка - ничего не делаем
}
}  // namespace client_lib

#endif  // CLIENT_LIB_STUB_H
