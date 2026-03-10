#include <iostream>

#include "client_lib.h"

using namespace client_lib;
using namespace std::literals;

class ClientEventHandler : public IClientEventHandler {
public:
    // handle received push
    void onPushReceived(uint32_t id, const Push& push) override {
        std::cout << "[Client] received: " << push.text << std::endl;
    }
    // handle accepted push by server
    void onAcceptPush(uint32_t id) override {
        std::cout << "[Client] server accept push with id: " << id << std::endl;
    }

    // handle sent data
    void onDataSend(std::size_t length) override {
        std::cout << "[Client] sent: " << length << " bytes" << std::endl;
    }
    // handle connection on
    void onConnected() override {
        std::cout << "[App] Successfully connected to server" << std::endl;
    }
    // handle disconnection
    void onDisconnected() override {
        std::cout << "[App] Disconnected from server" << std::endl;
    }
    // handle reconnecting
    void onReconnect() override {
        std::cout << "[Client] reconnect in 3 seconds..." << std::endl;
    }
    // handle errors
    void onError(const std::string& error) override {
        std::cout << "[Client] " << error << std::endl;
    }
};

void InputLoop(ClientApp& client_app) {
    std::string command;
    std::cout << "Enter commands:\n"
              << "                connect (to connect with the server)\n"
              << "                disconnect (to disconnect from the server)\n"
              << "                login (to authentificate)\n"
              << "                send (to send message)\n"
              << "                restart (to restart client loop)\n"
              << "                stop (to stop client loop)\n"
              << "                quit (to quit)\n"
              << std::endl;

    uint32_t id = 1;

    while (true) {
        std::getline(std::cin, command);

        if (command.empty()) {
            continue;
        }

        if (command == "connect"s) {
            client_app.connect();
        } else if (command == "disconnect"s) {
            client_app.disconnect();
        } else if (command == "send"s) {
            std::string message;
            std::cout << "Message: ";
            std::getline(std::cin, message);
            client_app.sendMessage(
                id,
                Push(PushPriority::Normal, PushCategory::Notification, PushSource::Source1, message));
            ++id;
        } else if (command == "login"s) {
            std::string login;
            std::cout << "Login: ";
            std::getline(std::cin, login);
            std::string password;
            std::cout << "Password: ";
            std::getline(std::cin, password);
            client_app.login(login, password);
        } else if (command == "restart"s) {
            client_app.runClientAppLoop();
        } else if (command == "stop"s) {
            client_app.stopClientLoop();
        } else if (command == "quit") {
            break;
        } else {
            std::cout << "Unknown command. Use: connect, disconnect, login, send, restart, stop, quit" << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    using namespace std::literals;
    try {
        auto handler = std::make_shared<ClientEventHandler>();
        ClientApp client_app(handler);

        client_app.configure("localhost", "8080");

        client_app.runClientAppLoop();
        InputLoop(client_app);

    } catch (std::exception& e) {
        std::cerr << "[App] Exception: " << e.what() << std::endl;
    }
}
