#include "TcpSession.h"

#include <iostream>

namespace server {

ClientSession::ClientSession(tcp::socket socket, SessionId id) :
    socket_(std::move(socket)), id_(id), readBuffer_(1024) {
    std::cout << "[Session " << id_ << "] Created" << std::endl;
}

void ClientSession::setOnDisconnect(std::function<void(SessionId)> callback) {
    onDisconnect_ = std::move(callback);
}

void ClientSession::setOnDataReceived(OnDataReceivedCallback callback) {
    onDataReceived_ = std::move(callback);
}

void ClientSession::start() {
    std::cout << "[Session " << id_ << "] Started" << std::endl;
    doRead();
}

void ClientSession::asyncSend(const std::vector<uint8_t>& data) {
    auto self = shared_from_this();

    auto dataCopy = std::make_shared<std::vector<uint8_t>>(data);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(*dataCopy),
                             [this, self, dataCopy](boost::system::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     std::cout << "[Session " << id_ << "] Sent " << length << " bytes" << std::endl;
                                 } else {
                                     std::cerr << "[Session " << id_ << "] Send error: " << ec.message() << std::endl;
                                     close();
                                 }
                             });
}

void ClientSession::close() {
    if (socket_.is_open()) {
        std::cout << "[Session " << id_ << "] Closing" << std::endl;

        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);

        if (onDisconnect_) {
            onDisconnect_(id_);
        }
    }
}

void ClientSession::doRead() {
    auto self = shared_from_this();

    socket_.async_read_some(boost::asio::buffer(readBuffer_),
                            [this, self](boost::system::error_code ec, std::size_t length) {
                                if (!ec) {
                                    std::cout << "[Session " << id_ << "] Received " << length << " bytes: ";

                                    for (size_t i = 0; i < length; ++i) {
                                        std::cout << readBuffer_[i];
                                    }
                                    std::cout << std::endl;

                                    if (onDataReceived_) {
                                        std::vector<uint8_t> data(readBuffer_.begin(), readBuffer_.begin() + length);
                                        onDataReceived_(id_, data);
                                    }

                                    doRead();
                                } else {
                                    std::cerr << "[Session " << id_ << "] Read error: " << ec.message() << std::endl;
                                    close();
                                }
                            });
}

void ClientSession::doWrite() {
    auto self = shared_from_this();

    boost::asio::async_write(socket_,
                             boost::asio::buffer(writeBuffer_),
                             [this, self](boost::system::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     std::cout << "[Session " << id_ << "] sent " << length << " bytes" << std::endl;
                                 } else {
                                     std::cerr << "[Session " << id_ << "] Write error: " << ec.message() << std::endl;
                                     close();
                                 }
                             });
}

}  // namespace server
