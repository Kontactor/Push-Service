#include "admin_push_generator.h"

#include <iostream>

#include "random_push_factory.h"

AdminPushGenerator::AdminPushGenerator(client::TcpClient& client, std::string login, std::string password) :
    PushGenerator(client), login_(std::move(login)), password_(std::move(password)) {}

void AdminPushGenerator::SendAuthIfNeeded() {
    if (authorized_) {
        return;
    }

    Push authPush{PushPriority::High,
                  PushCategory::Notification,
                  PushSource::Source1,
                  "AUTH|" + login_ + "|" + password_};

    auto bytes = serializer_.Serialize(authPush);
    client_.asyncSend(bytes);

    std::cout << "[ADMIN] Authorization sent\n";

    authorized_ = true;
}

Push AdminPushGenerator::GeneratePush() {
    SendAuthIfNeeded();

    return RandomPushFactory::CreateAdminPush();
}
