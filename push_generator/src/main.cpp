#include <boost/asio.hpp>
#include <iostream>

#include "TcpClient.h"
#include "admin_push_generator.h"
#include "push_generator.h"
#include "random_push_factory.h"

class UserPushGenerator : public PushGenerator {
public:
    using PushGenerator::PushGenerator;

private:
    Push GeneratePush() override {
        return RandomPushFactory::CreateUserPush();
    }
};

int main() {
    boost::asio::io_context io;

    client::TcpClient client(io);
    client.setEndpoint("127.0.0.1", "8080");
    client.connect();

    std::unique_ptr<PushGenerator> generator;

    std::cout << "Start as admin [y/n]: ";

    if (std::cin.get() == 'y' || std::cin.get() == 'Y') {
        std::cout << "\nEnter admin password: ";
        std::string password;
        std::cin >> password;

        generator = std::make_unique<AdminPushGenerator>(client, "admin", "qwerty123");

    } else {
        generator = std::make_unique<UserPushGenerator>(client);
    }
    generator->Start();
    io.run();
    generator->Stop();
    client.close();
}
