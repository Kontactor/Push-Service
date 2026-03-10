#include "client_lib.h"

#include <format>

#include "serialization/binary_push_serializer.h"

namespace client_lib {

std::string_view clientTypeToString(ClientRole client_type) {
    switch (client_type) {
        case ClientRole::Admin:
            return "admin";
        case ClientRole::User:
            return "user";
        default:
            return "unknown";
    }
}

std::optional<ClientRole> stringToClientType(std::string_view client_type_str) {
    // TODO: case-insensitive version
    if (client_type_str == "admin") {
        return ClientRole::Admin;
    }
    if (client_type_str == "user") {
        return ClientRole::User;
    }
    return std::nullopt;
}

// function stub for authorization
std::optional<AuthData> authorizeClient(std::string_view username, std::string_view password) {
    if (username == ADMIN_TEST_LOGIN && password == ADMIN_TEST_PASSWORD) {
        return AuthData{"123456", ClientRole::Admin, 123};
    }

    if (username == USER_TEST_LOGIN && password == USER_TEST_PASSWORD) {
        return AuthData{"123456", ClientRole::User, 123};
    }

    return std::nullopt;
}

// ************************ ClientApp Methods *********************************

ClientApp::ClientApp(std::shared_ptr<IClientEventHandler> handler) :
    handler_(handler), tcp_client_(std::make_unique<client::TcpClient>(io_context_)),
    protocol_(std::make_unique<protocol::ProtocolAdapter>(std::make_unique<BinaryPushSerializer>())) {
    setTcpEventHandlers();
}

ClientApp::~ClientApp() {
    stopClientLoop();
}

void ClientApp::configure(std::string host, std::string port) {
    host_ = std::move(host);
    port_ = std::move(port);
}

// run client loop
void ClientApp::runClientAppLoop() {
    if (!io_thread_) {
        io_thread_ = std::make_unique<std::jthread>([this]() {
            work_guard_ = std::make_unique<GuardType>(boost::asio::make_work_guard(io_context_));
            if (io_context_.stopped()) {
                io_context_.restart();
            }
            io_context_.run();
        });
    }
}

// stop client loop
void ClientApp::stopClientLoop() {
    if (io_thread_) {
        work_guard_->reset();
        disconnect();
        io_context_.stop();
        io_thread_->join();
        io_thread_.reset();
    }
}

void ClientApp::connect() {
    if (isConnected()) {
        handler_->onError("Already connected");
        return;
    }

    if (configureTcpClient()) {  // set connection params for tcp client
        tcp_client_->connect();
    }
}

void ClientApp::disconnect() {
    if (isConnected()) {
        tcp_client_->close();
        user_data_.reset();
    }
}

std::optional<ClientRole> ClientApp::login(const std::string& login, const std::string& password) {
    if (!isConnected()) {
        handler_->onError("First you need to connect to the server");
        return std::nullopt;
    }

    try {
        user_data_ = requestRoleFromServer(login, password);

        if (!user_data_) {
            return std::nullopt;
        }

        return user_data_->role;
    } catch (const std::exception& e) {
        handler_->onError(std::string("Login failed: ") + e.what());
        return std::nullopt;
    }
}

bool ClientApp::canSend() const {
    return user_data_.has_value() && user_data_->role == ClientRole::Admin;
}

// only for admin
void ClientApp::sendMessage(uint32_t id, const Push& push) {
    if (!isConnected()) {
        handler_->onError("First you need to connect to the server");
        return;
    }

    if (canSend()) {
        auto frame = protocol_->SerializeToFrame(push, id, protocol::MessageType::Push);

        tcp_client_->asyncSend(frame);
    } else {
        handler_->onError("Only administrator can send messages");
    }
}

std::optional<ClientRole> ClientApp::getRole() const {
    if (user_data_) {
        return user_data_->role;
    }

    return std::nullopt;
}

// return connection info of server in format "host:port"
std::string ClientApp::connectionInfo() const {
    std::string connection_info = std::format("{}:{}", host_, port_);
    return connection_info;
}

void ClientApp::onDataReceived(const std::vector<uint8_t>& data) {
    if (auto message = protocol_->Feed(data); message.has_value()) {
        switch (message->type) {
            case protocol::MessageType::Push:
                if (message->push) {
                    handler_->onPushReceived(message->id, message->push.value());
                }
                break;
            case protocol::MessageType::Accepted:
                handler_->onAcceptPush(message->id);
                break;
            case protocol::MessageType::Error:
                handler_->onError("Server error");
                break;
            default:
                handler_->onError("Unknown type of message");
        }
    }
}

bool ClientApp::isConnected() const {
    return tcp_client_->isConnected();
}

std::optional<AuthData> ClientApp::requestRoleFromServer(const std::string& login, const std::string& password) {
    // TODO: implement request to server
    // authentication stub
    // std::string auth_query = std::format("AUTH:{}:{}", login, password);
    // std::vector<uint8_t> auth_data(auth_query.begin(), auth_query.end());

    // tcp_client_->asyncSend(auth_data);

    return authorizeClient(login, password);
}

// set host an port for tcp connection in tcp client
bool ClientApp::configureTcpClient() {
    if (host_.empty() || port_.empty()) {
        handler_->onError("Connection params doesn't set");
        return false;
    }

    tcp_client_->setEndpoint(host_, port_);
    return true;
}

// set tcp client event handlers
void ClientApp::setTcpEventHandlers() {
    tcp_client_->setOnConnected([this]() {
        handler_->onConnected();
    });

    tcp_client_->setOnDisconnected([this]() {
        handler_->onDisconnected();
    });

    tcp_client_->setOnReconnect([this]() {
        handler_->onReconnect();
    });

    tcp_client_->setOnDataReceived([this](const std::vector<uint8_t>& data) {
        onDataReceived(data);
    });

    tcp_client_->setOnDataSend([this](size_t length) {
        handler_->onDataSend(length);
    });

    tcp_client_->setOnError([this](const std::string& error) {
        handler_->onError(error);
    });
}

}  // namespace client_lib
