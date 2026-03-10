#include "TcpClient.h"

#include <chrono>

namespace client {

TcpClient::TcpClient(boost::asio::io_context& io_context) :
    io_context_(io_context), socket_(io_context), resolver_(io_context), last_op_success_(false), was_connected_(false),
    reconnect_count_(0), readBuffer_(1024), shouldReconnect_(false), reconnectTimer_(io_context) {}

void TcpClient::setOnConnected(std::function<void()> callback) {
    onConnected_ = std::move(callback);
}

void TcpClient::setOnDisconnected(std::function<void()> callback) {
    onDisconnected_ = std::move(callback);
}

void TcpClient::setOnReconnect(std::function<void()> callback) {
    onReconnect_ = std::move(callback);
}

void TcpClient::setOnDataReceived(std::function<void(const std::vector<uint8_t>&)> callback) {
    onDataReceived_ = std::move(callback);
}

void TcpClient::setOnDataSend(std::function<void(size_t)> callback) {
    onDataSend_ = std::move(callback);
}

void TcpClient::setOnError(std::function<void(const std::string&)> callback) {
    onError_ = std::move(callback);
}

void TcpClient::setEndpoint(const std::string& host, const std::string& port) {
    was_connected_ = false;
    shouldReconnect_ = false;
    boost::system::error_code ec;
    endpoints_ = resolver_.resolve(host, port, ec);
    last_op_success_ = !ec;

    if (ec) {
        if (onError_) {
            onError_("Error when resolving of an endpoint: " + ec.message());
        }
    }
}

bool TcpClient::isConnected() const {
    return socket_.is_open() && !io_context_.stopped() && last_op_success_;
}

void TcpClient::connect() {
    if (last_op_success_) {
        doConnect();
    }
}

void TcpClient::doConnect() {
    boost::asio::async_connect(socket_, endpoints_, [this](boost::system::error_code ec, tcp::endpoint) {
        last_op_success_ = !ec;
        if (!ec) {
            if (onConnected_) {
                onConnected_();
            }
            shouldReconnect_ = true;
            was_connected_ = true;
            reconnect_count_ = 0;
            doRead();
        } else {
            if (onError_) {
                onError_("Connect error");
            }

            if (shouldReconnect_) {
                scheduleReconnect();
            }
        }
    });
}

void TcpClient::doRead() {
    socket_.async_read_some(boost::asio::buffer(readBuffer_), [this](boost::system::error_code ec, std::size_t length) {
        last_op_success_ = !ec;
        if (!ec) {
            if (onDataReceived_) {
                std::vector<uint8_t> data(readBuffer_.begin(), readBuffer_.begin() + length);
                onDataReceived_(data);
            }

            doRead();
        } else {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (onError_) {
                onError_("Read error: " + ec.message());
            }

            closeConnection();

            if (shouldReconnect_) {
                scheduleReconnect();
            }
        }
    });
}

void TcpClient::asyncSend(const std::vector<uint8_t>& data) {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(data),
                             [this, data](boost::system::error_code ec, std::size_t length) {
                                 last_op_success_ = !ec;
                                 if (!ec) {
                                     if (onDataSend_) {
                                         onDataSend_(length);
                                     }
                                 } else {
                                     if (onError_) {
                                         onError_("Send error: " + ec.message());
                                     }
                                 }
                             });
}

void TcpClient::scheduleReconnect() {
    ++reconnect_count_;

    if (onReconnect_) {
        onReconnect_();
    }

    reconnectTimer_.expires_after(std::chrono::seconds(3));
    reconnectTimer_.async_wait([this](boost::system::error_code ec) {
        if (!ec) {
            doConnect();
        }
    });

    if (reconnect_count_ == reconnect_max_count_) {
        shouldReconnect_ = false;
        reconnect_count_ = 0;
    }
}

void TcpClient::closeConnection() {
    boost::system::error_code ec;

    socket_.cancel(ec);
    if (ec) {
        onError_("Error when canceling operations: " + ec.message());
    }

    socket_.shutdown(tcp::socket::shutdown_both, ec);
    if (ec) {
        onError_("Shutdown error: " + ec.message());
    }

    socket_.close(ec);
    if (ec) {
        onError_("Error closing the socket: " + ec.message());
    }

    if (onDisconnected_) {
        onDisconnected_();
    }
}

void TcpClient::close() {
    shouldReconnect_ = false;

    closeConnection();

    reconnectTimer_.cancel();
}

}  // namespace client
