#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace client {

using boost::asio::ip::tcp;

class TcpClient {
public:
    TcpClient(boost::asio::io_context& io_context);

    void setEndpoint(const std::string& host, const std::string& port);
    bool isConnected() const;

    void connect();
    void asyncSend(const std::vector<uint8_t>& data);
    void close();

    void setOnConnected(std::function<void()> callback);
    void setOnDisconnected(std::function<void()> callback);
    void setOnReconnect(std::function<void()> callback);
    void setOnDataReceived(std::function<void(const std::vector<uint8_t>&)> callback);
    void setOnDataSend(std::function<void(size_t)> callback);
    void setOnError(std::function<void(const std::string&)> callback);

private:
    void doConnect();
    void doRead();
    void scheduleReconnect();
    void closeConnection();

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    tcp::resolver resolver_;
    tcp::resolver::results_type endpoints_;

    // is the last network operation successful
    bool last_op_success_;
    bool was_connected_;
    int reconnect_count_;
    const int reconnect_max_count_ = 3;

    std::vector<uint8_t> readBuffer_;

    bool shouldReconnect_;
    boost::asio::steady_timer reconnectTimer_;

    std::function<void()> onConnected_;
    std::function<void()> onDisconnected_;
    std::function<void()> onReconnect_;
    std::function<void(const std::vector<uint8_t>&)> onDataReceived_;
    std::function<void(size_t)> onDataSend_;
    std::function<void(const std::string&)> onError_;
};

}  // namespace client
