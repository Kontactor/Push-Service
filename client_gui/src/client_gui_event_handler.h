// HEADER для создание заглушек клиентской библиотеки
#ifndef CLIENT_GUI_EVENT_HANDLER_H
#define CLIENT_GUI_EVENT_HANDLER_H

#include <QObject>
#include <string>

#include "client_lib.h"

class ClientGUIEventHandler : public QObject, public client_lib::IClientEventHandler {
    Q_OBJECT

public:
    ClientGUIEventHandler(QObject* parent = nullptr) : QObject(parent) {}
    ~ClientGUIEventHandler() override = default;

    // handle received push
    void onPushReceived(uint32_t id, const Push& push) override {
        emit pushReceived(id, push);
    }
    // handle accepted push by server
    void onAcceptPush(uint32_t id) override {
        emit acceptPush(id);
    }
    // handle connection on
    void onConnected() override {
        emit connected();
    }
    // handle disconnection
    void onDisconnected() override {
        emit disconnected();
    }
    // handle reconnecting
    void onReconnect() override {
        emit reconnect();
    }
    // handle sended data
    void onDataSend(std::size_t size) override {
        emit dataSend(size);
    }
    // handle errors
    void onError(const std::string& error) override {
        QString decodedString = QString::fromLocal8Bit(error.c_str(), error.length());
        emit errorOccurred(decodedString);
    }

signals:
    void pushReceived(uint32_t id, const Push& push);
    void acceptPush(uint32_t id);
    void connected();
    void disconnected();
    void reconnect();
    void dataSend(std::size_t size);
    void errorOccurred(const QString& error);
};

#endif  // CLIENT_GUI_EVENT_HANDLER_H
