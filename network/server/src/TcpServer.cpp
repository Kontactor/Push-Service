#include "TcpServer.h"

#include <iostream>

namespace server {

TcpServer::TcpServer(boost::asio::io_context& io_context, unsigned short port) :
    io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), nextSessionId_(1) {
    std::cout << "[Server] Created on port " << port << std::endl;
}

void TcpServer::setCallbacks(OnSessionConnectedCallback onConnected,
                             OnSessionDisconnectedCallback onDisconnected,
                             OnDataReceivedCallback onDataReceived) {
    onConnected_ = std::move(onConnected);
    onDisconnected_ = std::move(onDisconnected);
    onDataReceived_ = std::move(onDataReceived);
}

void TcpServer::start() {
    std::cout << "[Server] Starting..." << std::endl;
    doAccept();
}

void TcpServer::stop() {
    boost::asio::post(io_context_, [this] {
        std::cout << "[Server] Stopping..." << std::endl;

        boost::system::error_code ec;
        acceptor_.close(ec);
        if (ec) {
            std::cerr << "[Server] Acceptor close error: " << ec.message() << std::endl;
        }

        client_registry_.ClearAll();
    });
}

void TcpServer::broadcast(const std::vector<uint8_t>& data) {
    auto send_msg = [&](SessionId id, const std::shared_ptr<ClientSession>& client) {
        std::cout << "[Server] Sending to session " << id << "\n";
        client->asyncSend(data);
    };

    client_registry_.ForEachClient(send_msg);
}

void TcpServer::removeSession(SessionId id) {
    std::cout << "[Server] Removing session " << id << std::endl;
    client_registry_.Remove(id);

    if (onDisconnected_) {
        onDisconnected_(id);
    }
}

void TcpServer::doAccept() {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto endpoint = socket.remote_endpoint();
            std::cout << "[Server] New connection from " << endpoint.address().to_string() << ":" << endpoint.port()
                      << std::endl;

            SessionId id = nextSessionId_++;
            auto session = std::make_shared<ClientSession>(std::move(socket), id);

            session->setOnDisconnect([this](SessionId sid) {
                removeSession(sid);
            });

            session->setOnDataReceived(onDataReceived_);

            client_registry_.Add(session);

            if (onConnected_) {
                onConnected_(id, session);
            }

            session->start();
        } else {
            std::cerr << "[Server] Accept error: " << ec.message() << std::endl;
        }

        if (acceptor_.is_open()) {
            doAccept();
        }
    });
}

size_t TcpServer::ActiveClientsCount() const {
    return client_registry_.ActiveClientsCount();
}

}  // namespace server
