#include "../include/server.h"

Server::~Server() {
    StopBroadcast();
};

void Server::StopBroadcast() {
    dispatcher_->Stop();
}

void Server::PostPushNotification(std::shared_ptr<IPush> push) {
    dispatcher_->Post(std::move(push));
}
