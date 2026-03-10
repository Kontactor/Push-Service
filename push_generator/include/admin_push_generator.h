#pragma once

#include <string>

#include "push_generator.h"

class AdminPushGenerator : public PushGenerator {
public:
    AdminPushGenerator(client::TcpClient& client, std::string login, std::string password);

protected:
    Push GeneratePush() override;

private:
    void SendAuthIfNeeded();

private:
    std::string login_;
    std::string password_;
    bool authorized_{false};
};
