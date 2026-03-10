#pragma once

#include <boost/asio.hpp>
#include <memory>

#include "Types.h"

namespace server {
using boost::asio::ip::tcp;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(tcp::socket socket, SessionId id);

    void start();
    void asyncSend(const std::vector<uint8_t>& data);
    void close();

    SessionId getId() const {
        return id_;
    }
    bool isConnected() const {
        return socket_.is_open();
    }

    void setOnDisconnect(std::function<void(SessionId)> callback);
    void setOnDataReceived(OnDataReceivedCallback callback);

private:
    void doRead();
    void doWrite();

    tcp::socket socket_;
    SessionId id_;
    std::vector<uint8_t> readBuffer_;
    std::vector<uint8_t> writeBuffer_;

    std::function<void(SessionId)> onDisconnect_;
    OnDataReceivedCallback onDataReceived_;
};
}  // namespace server
