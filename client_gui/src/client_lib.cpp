// client_lib.cpp

#include "client_lib.h"

namespace client_lib {

UserClient::UserClient(boost::asio::io_context& ioContext, std::shared_ptr<IPushMessageHandler> handler) :
    ioContext_(ioContext), socket_(ioContext), handler_(handler) {}

void UserClient::connect(const std::string& host, int port) {
    tcp::resolver resolver(ioContext_);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    boost::asio::async_connect(socket_, endpoints, [this](const boost::system::error_code& ec, const tcp::endpoint&) {
        if (!ec) {
            handler_->onConnectionStatusChanged(true);
            connected_ = true;
            startRead();
        } else {
            handler_->onError(ec.message());
        }
    });
}

void UserClient::disconnect() {
    if (connected_) {
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
        connected_ = false;
        handler_->onConnectionStatusChanged(false);
    }
}

void UserClient::startRead() {
    if (!connected_)
        return;

    socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            std::string message(buffer_.data(), length);
            handler_->onPushMessage(message);

            startRead();
        } else {
            connected_ = false;
            handler_->onError(ec.message());
        }
    });
}

void UserClient::sendMessage(const std::string& payload) {
    if (connected_) {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(payload),
                                 [this](const boost::system::error_code& ec, std::size_t) {
                                     if (ec) {
                                         handler_->onError(ec.message());
                                     }
                                 });
    }
}

void UserClient::sendPush(const std::string& payload) {
    // Exeption or warning if not admin?
    throw std::runtime_error("User client doesn't support sendPush");
}

void AdminClient::sendPush(const std::string& payload) {
    sendMessage("PUSH" + payload);
}

std::optional<ClientType> authorizeClient(std::string username, std::string password) {
    std::optional<ClientType> result;

    if (username == ADMIN_TEST_LOGIN && password == ADMIN_TEST_PASSWORD) {
        return ClientType::Admin;
    }

    if (username == USER_TEST_LOGIN && password == USER_TEST_PASSWORD) {
        return ClientType::User;
    }

    return result;
}

void userInputLoop(IPushClient& client) {
    std::string input;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        if (input.size() != 0) {
            std::cout << "You are in userLoop, type \"exit\" to quit" << std::endl;
        }
    }
}

std::unique_ptr<IPushClient> UserClientFactory::createUser(boost::asio::io_context& ioContext,
                                                           std::shared_ptr<IPushMessageHandler> handler,
                                                           const AuthResponse& userdata) {
    if (userdata.role == "admin") {
        return std::make_unique<AdminClient>(ioContext, handler);
    } else if (userdata.role == "user") {
        return std::make_unique<UserClient>(ioContext, handler);
    } else {
        throw std::invalid_argument("Unknown role: " + userdata.role);
    }
}

bool ClientApp::login(const std::string& login, const std::string& password) {
    try {
        auto userdata = requestRoleFromServer(login, password);

        currentUser_ = UserClientFactory::createUser(*ioContext_, handler_, userdata);

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Login failed: " << e.what() << std::endl;
        return false;
    }
}

IPushClient* ClientApp::getCurrentUser() {
    if (currentUser_) {
        return currentUser_.get();
    } else {
        return nullptr;
    }
}

AuthResponse ClientApp::requestRoleFromServer(const std::string& login, const std::string& password) {
    // TODO: implement request to server
    return AuthResponse{"123456", "user", 123};
}

}  // namespace client_lib
