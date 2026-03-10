#pragma once

#include <optional>
#include <stdexcept>
#include <thread>

#include "TcpClient.h"
#include "protocol.h"

namespace client_lib {

using namespace client;

const std::string ADMIN_TEST_LOGIN = "admin";
const std::string ADMIN_TEST_PASSWORD = "admin";
const std::string USER_TEST_LOGIN = "user";
const std::string USER_TEST_PASSWORD = "user";

enum class ClientRole { Admin, User };

struct AuthData {
    std::string token;
    ClientRole role;
    int user_id;
};

// interface to handle client events
class IClientEventHandler {
public:
    virtual ~IClientEventHandler() = default;

    // handle received push
    virtual void onPushReceived(uint32_t id, const Push& push) = 0;
    // handle accepted push by server
    virtual void onAcceptPush(uint32_t id) = 0;

    // handle connection on
    virtual void onConnected() = 0;
    // handle disconnection
    virtual void onDisconnected() = 0;
    // handle reconnecting
    virtual void onReconnect() = 0;
    // handle sent data
    virtual void onDataSend(std::size_t) = 0;
    // handle errors
    virtual void onError(const std::string& error) = 0;
};

std::string_view clientTypeToString(ClientRole client_type);

std::optional<ClientRole> stringToClientType(std::string_view client_type_str);

std::optional<AuthData> authorizeClient(std::string_view username, std::string_view password);

class ClientApp {
public:
    using GuardType = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    explicit ClientApp(std::shared_ptr<IClientEventHandler> handler);

    ~ClientApp();
    // connection params: host and port
    void configure(std::string host, std::string port);

    // run client loop
    void runClientAppLoop();
    // stop client loop
    void stopClientLoop();

    void connect();

    void disconnect();

    std::optional<ClientRole> login(const std::string& login, const std::string& password);
    // check if the client can send the message to the server.
    [[nodiscard]] bool canSend() const;

    void sendMessage(uint32_t id, const Push& push);  // only for admin

    [[nodiscard]] std::optional<ClientRole> getRole() const;
    // return connection info of server in format "host:port"
    [[nodiscard]] std::string connectionInfo() const;

private:
    boost::asio::io_context io_context_{};
    std::shared_ptr<IClientEventHandler> handler_;

    std::unique_ptr<client::TcpClient> tcp_client_;

    std::unique_ptr<GuardType> work_guard_;

    std::unique_ptr<protocol::ProtocolAdapter> protocol_;

    std::string host_;
    std::string port_;

    std::optional<AuthData> user_data_;

    std::unique_ptr<std::jthread> io_thread_;

private:
    // handle received data
    void onDataReceived(const std::vector<uint8_t>& data);

    bool isConnected() const;
    // Request role from server
    std::optional<AuthData> requestRoleFromServer(const std::string& login, const std::string& password);
    // set host an port for tcp connnection in tcp client
    bool configureTcpClient();
    // set tcp client event handlers
    void setTcpEventHandlers();
};

}  // namespace client_lib
