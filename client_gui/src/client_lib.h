// client_lib.h

#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <optional>
#include <string>

namespace client_lib {

namespace net = boost::asio;

using tcp = net::ip::tcp;

const std::string ADMIN_TEST_LOGIN = "admin";
const std::string ADMIN_TEST_PASSWORD = "admin";
const std::string USER_TEST_LOGIN = "user";
const std::string USER_TEST_PASSWORD = "user";

enum ClientType { Unknown, Admin, User };

// Interface for push message handler
class IPushMessageHandler {
public:
    virtual ~IPushMessageHandler() = default;

    // handle push message
    virtual void onPushMessage(const std::string& message) = 0;

    // handle connection status changed
    virtual void onConnectionStatusChanged(bool connected) = 0;

    // handle errors
    virtual void onError(const std::string& error) = 0;
};

// Interface client class
class IPushClient : public std::enable_shared_from_this<IPushClient> {
public:
    virtual ~IPushClient() = default;

    virtual void connect(const std::string& host, int port) = 0;
    virtual void disconnect() = 0;

    virtual void startRead() = 0;

    virtual void sendMessage(const std::string& payload) = 0;
    virtual void sendPush(const std::string& payload) = 0;
};

// User client class
class UserClient : public IPushClient {
public:
    UserClient(boost::asio::io_context& ioContext, std::shared_ptr<IPushMessageHandler> handler);

    void connect(const std::string& host, int port) override;
    void disconnect() override;

    void startRead() override;

    void sendMessage(const std::string& payload) override;
    void sendPush(const std::string& payload) override;

private:
    // void onRead(...); - read smth

    boost::asio::io_context& ioContext_;
    tcp::socket socket_;
    std::shared_ptr<IPushMessageHandler> handler_;
    bool connected_ = false;
    std::array<char, 1024> buffer_;
};

// Admin client class
class AdminClient : public UserClient {
public:
    AdminClient(boost::asio::io_context& ioContext, std::shared_ptr<IPushMessageHandler> handler) :
        UserClient(ioContext, handler) {}

    void sendPush(const std::string& payload) override;
};

std::optional<ClientType> authorizeClient(std::string username, std::string password);

// Base for User input loop
void userInputLoop(IPushClient& client);

struct AuthResponse {
    std::string token;
    std::string role;
    int user_id;
};

class UserClientFactory {
public:
    static std::unique_ptr<IPushClient> createUser(boost::asio::io_context& ioContext,
                                                   std::shared_ptr<IPushMessageHandler> handler,
                                                   const AuthResponse& userdata);
};

class ClientApp {
public:
    ClientApp(std::shared_ptr<IPushMessageHandler> handler) :
        ioContext_(std::make_unique<boost::asio::io_context>()), handler_(handler) {}

    bool login(const std::string& login, const std::string& password);

    IPushClient* getCurrentUser();

private:
    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<IPushClient> currentUser_;
    std::shared_ptr<IPushMessageHandler> handler_;

    // Request role from server
    AuthResponse requestRoleFromServer(const std::string& login, const std::string& password);
};

}  // namespace client_lib
