#include <iostream>
#include <string>

#include "TcpServer.h"

int main() {
    try {
        boost::asio::io_context io_context;

        server::TcpServer server(io_context, 8080);

        server.setCallbacks(
            [&server](server::SessionId id, auto session) {
                std::cout << "[App] Client " << id << " connected" << std::endl;

                std::string message = "12345";
                std::vector<uint8_t> data(message.begin(), message.end());
                server.broadcast(data);
            },
            [](server::SessionId id) {
                std::cout << "[App] Client " << id << " disconnected" << std::endl;
            },
            [](server::SessionId id, const std::vector<uint8_t>& data) {
                std::cout << "[App] Data from client " << id << ": ";
                for (auto byte : data) {
                    std::cout << byte;
                }
                std::cout << std::endl;
            });

        server.start();

        std::cout << "[App] Server started on port 8080" << std::endl;
        std::cout << "[App] Press Ctrl+C to stop" << std::endl;

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "[App] Exception: " << e.what() << std::endl;
    }

    return 0;
}
