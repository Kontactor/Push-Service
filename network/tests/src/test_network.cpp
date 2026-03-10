#include <gtest/gtest.h>
//
#include <boost/asio.hpp>
#include <thread>

#include "ClientRegistry.h"
#include "TcpClient.h"
#include "TcpServer.h"

namespace net = boost::asio;
using namespace std::chrono_literals;
using namespace std::string_literals;

namespace test_cfg {
const std::string host = "127.0.0.1"s;
constexpr uint16_t port = 18080;
const std::string port_str = std::to_string(port);
}  // namespace test_cfg

inline void Wait() {
    std::this_thread::sleep_for(300ms);
}

TEST(Broadcast, ReceiveMessage) {
    // 01. Server: start
    net::io_context server_ioc;
    server::TcpServer server(server_ioc, test_cfg::port);
    server.start();
    std::thread server_thread([&] {
        server_ioc.run();
    });

    // 02. Add Client
    net::io_context client_ioc;
    std::string last_msg;
    client::TcpClient client(client_ioc);
    client.setEndpoint(test_cfg::host, test_cfg::port_str);
    client.setOnDataReceived([&](const std::vector<uint8_t>& data) {
        last_msg.assign(data.begin(), data.end());
    });
    client.connect();
    std::thread client_thread([&] {
        client_ioc.run();
    });
    Wait();
    EXPECT_EQ(server.ActiveClientsCount(), 1u);

    // 03. Test: Send Message
    std::string test_msg = "12345";
    std::vector<uint8_t> data(test_msg.begin(), test_msg.end());
    server.broadcast(data);
    Wait();
    EXPECT_EQ(last_msg, test_msg);

    // 04. Server: stop
    server.stop();
    Wait();
    EXPECT_EQ(server.ActiveClientsCount(), 0u);

    // 05. Cleanup
    client_ioc.stop();
    server_ioc.stop();
    client_thread.join();
    server_thread.join();
}

TEST(ClientRegistry, ConnectingClientsToServer) {
    // 01. Server: start
    net::io_context server_ioc;
    server::TcpServer server(server_ioc, test_cfg::port);
    server.start();
    std::thread server_thread([&] {
        server_ioc.run();
    });

    // 02. Add Client
    net::io_context client_ioc;
    client::TcpClient c1(client_ioc);
    c1.setEndpoint(test_cfg::host, test_cfg::port_str);
    c1.connect();
    std::thread client_thread([&] {
        client_ioc.run();
    });
    Wait();
    EXPECT_EQ(server.ActiveClientsCount(), 1u);

    // 02. Add Another Clients
    client::TcpClient c2(client_ioc);
    c2.setEndpoint(test_cfg::host, test_cfg::port_str);
    c2.connect();
    client::TcpClient c3(client_ioc);
    c3.setEndpoint(test_cfg::host, test_cfg::port_str);
    c3.connect();
    Wait();
    EXPECT_EQ(server.ActiveClientsCount(), 3u);

    // 03. Close one client
    c2.close();
    Wait();
    EXPECT_EQ(server.ActiveClientsCount(), 2u);

    // 04. Server: stop
    server.stop();
    Wait();
    EXPECT_EQ(server.ActiveClientsCount(), 0u);

    // 05. Cleanup
    client_ioc.stop();
    server_ioc.stop();
    client_thread.join();
    server_thread.join();
}

TEST(ClientRegistryTest, WritingRace) {
    server::ClientRegistry cl_registry;

    server::SessionId id = 666;
    boost::asio::io_context io_context;
    server::tcp::socket socket(io_context);
    auto session = std::make_shared<server::ClientSession>(std::move(socket), id);

    std::thread t1([&cl_registry, &session]() {
        for (int i = 0; i < 10; ++i) {
            cl_registry.Add(session);
        }
    });

    std::thread t2([&cl_registry, &session]() {
        for (int i = 0; i < 10; ++i) {
            cl_registry.Add(session);
        }
    });

    t1.join();
    t2.join();

    ASSERT_TRUE(cl_registry.Contains(id));
    ASSERT_EQ(cl_registry.ActiveClientsCount(), 1);
}

TEST(ClientRegistryTest, CorrectAddRemove) {
    server::ClientRegistry cl_registry;

    boost::asio::io_context io_context;

    const int count = 10;

    for (int i = 0; i < count; ++i) {
        server::tcp::socket socket(io_context);
        auto session = std::make_shared<server::ClientSession>(std::move(socket), i);
        cl_registry.Add(session);
    }

    std::thread t1([&cl_registry]() {
        for (int i = 0; i < count; ++i) {
            cl_registry.Remove(i);
        }
    });

    std::thread t2([&cl_registry, &io_context]() {
        for (int i = count; i < 2 * count; ++i) {
            server::tcp::socket socket(io_context);
            auto session = std::make_shared<server::ClientSession>(std::move(socket), i);
            cl_registry.Add(session);
        }
    });

    t1.join();
    t2.join();

    ASSERT_EQ(cl_registry.ActiveClientsCount(), count);
}
