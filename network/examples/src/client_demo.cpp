#include <csignal>
#include <iostream>

#include "TcpClient.h"

boost::asio::io_context* g_io_context = nullptr;

void signalHandler(int signal) {
    if (g_io_context) {
        std::cout << "\n[App] Shutting down..." << std::endl;
        g_io_context->stop();
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        g_io_context = &io_context;

        std::signal(SIGINT, signalHandler);

        client::TcpClient client(io_context);

        client.setEndpoint("localhost", "8080");

        client.setOnConnected([]() {
            std::cout << "[App] Successfully connected to server" << std::endl;
        });

        client.setOnDisconnected([]() {
            std::cout << "[App] Disconnected from server" << std::endl;
        });

        client.setOnDataReceived([](const std::vector<uint8_t>& data) {
            std::cout << "[App] Received message: ";
            for (auto byte : data) {
                std::cout << static_cast<char>(byte);
            }
            std::cout << std::endl;
        });

        client.connect();

        std::cout << "[App] Client started. Press Ctrl+C to stop" << std::endl;

        io_context.run();

        client.close();
    } catch (std::exception& e) {
        std::cerr << "[App] Exception: " << e.what() << std::endl;
    }

    return 0;
}
