#pragma once

#include <vector>

#include "dispatcher.h"

using ClientsQueue = std::deque<std::shared_ptr<IClient>>;

// imitation of client class
class Client : public IClient {
public:
    explicit Client(int id) : id_{id} {}

    void SendPush(std::shared_ptr<IPush> push) override {
        received_pushes_.push_back(*push);
    }

    std::vector<IPush> GetReceivedPushes() const override {
        return received_pushes_;
    }

    int GetId() const override {
        return id_;
    }

private:
    int id_;
    std::vector<IPush> received_pushes_;
};

// imitation of server class
class Server {
public:
    Server(const ClientsQueue& clients) : clients_(clients), dispatcher_(dispatcher::makeDispatcher(clients)) {}

    ~Server();

    void StopBroadcast();

    void PostPushNotification(std::shared_ptr<IPush> push);

private:
    const ClientsQueue& clients_;
    std::unique_ptr<dispatcher::IDispatcher> dispatcher_;
};
