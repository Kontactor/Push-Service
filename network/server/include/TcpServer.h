#pragma once

#include <boost/asio.hpp>
#include <vector>

#include "ClientRegistry.h"
#include "TcpSession.h"
#include "Types.h"

namespace server {
using boost::asio::ip::tcp;

class TcpServer {
public:
    TcpServer(boost::asio::io_context& io_context, unsigned short port);

    void setCallbacks(OnSessionConnectedCallback onConnected,
                      OnSessionDisconnectedCallback onDisconnected,
                      OnDataReceivedCallback onDataReceived);

    void start();
    void stop();

    void broadcast(const std::vector<uint8_t>& data);

    size_t ActiveClientsCount() const;

private:
    void doAccept();
    void removeSession(SessionId id);

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    SessionId nextSessionId_;

    ClientRegistry client_registry_;

    OnSessionConnectedCallback onConnected_;
    OnSessionDisconnectedCallback onDisconnected_;
    OnDataReceivedCallback onDataReceived_;
};
}  // namespace server
