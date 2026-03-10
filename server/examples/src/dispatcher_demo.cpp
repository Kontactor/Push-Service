#include <algorithm>
#include <format>
#include <iostream>
#include <thread>

#include "server.h"

using namespace std::literals;
using namespace dispatcher;

// =====================
// usefull functions for testing
// =====================
IPush GetRandomPush() {
    const std::vector<std::string> payloads = {"'Important': \"Check\" your 'security' settings"s,
                                               "'Reminder': It's \"time\" to complete the task"s,
                                               "Your 'order' has been shipped"s,
                                               "A gift awaits: \"activate\" the 'coupon'"s,
                                               "Be 'proud' of yourself — you tried"s,
                                               "Inspiration of the \"day\": a quote for you"s,
                                               "Your 'action' is waiting for confirmation!"s,
                                               "New message — open now"s,
                                               "You missed the fun — come back!"s,
                                               "The update is available — install it now"s,
                                               "Thank you for your trust! We appreciate you"s,
                                               "You have a new notification, take a look"s,
                                               "30% discount only today — don't miss it!"s,
                                               "Product in stock: order now"s,
                                               "We recommend: products according to your 'interests'"s,
                                               "New subscriber — meet me!"s,
                                               "You were mentioned in a 'comment'"s,
                                               "The payment was successful"s,
                                               "The card limit has been 'updated'"s,
                                               "Cashback is waiting: spend it now"s};
    return IPush{static_cast<PushType>(rand() % 2),
                 static_cast<PushPriority>(rand() % 3),
                 payloads[rand() % payloads.size()],
                 "Admin"s};
}

std::ostream& operator<<(std::ostream& out, const IPush& push) {
    std::string res = std::format("Type = {}, Priority = {}, Payload = {}, Sender = {}",
                                  static_cast<int>(push.type),
                                  static_cast<int>(push.priority),
                                  push.payload,
                                  push.sender);
    out << res;
    return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& pushes) {
    for (auto push : pushes) {
        out << push << std::endl;
    }
    return out;
}

// Create deque of clients with size equal to num
ClientsQueue CreateClients(const uint32_t num) {
    std::deque<std::shared_ptr<IClient>> clients;
    for (uint32_t i = 0; i < num; ++i) {
        clients.emplace_back(std::make_shared<Client>(i));
    }
    return clients;
}

// Create vector with pushes with size equal to num
std::vector<IPush> CreatePushes(const uint32_t num) {
    std::vector<IPush> pushes;
    pushes.reserve(num);
    for (uint32_t i = 0; i < num; ++i) {
        pushes.emplace_back(std::move(GetRandomPush()));
    }
    return pushes;
}

int main(int argc, char** argv) {
    const int clients_num = 3;
    // set clients (they will receive pushes) for server
    auto clients = CreateClients(clients_num);
    // create vector and fill with pushes
    const int pushes_num = 10;
    std::vector<IPush> pushes = CreatePushes(pushes_num);
    {
        Server server(clients);
        // start send pushes
        for (auto& push : pushes) {
            server.PostPushNotification(std::make_shared<IPush>(push));
        }
    }
    std::cout << "----------------------------------------------------------------------------\n" << std::endl;

    std::cout << "\nResults:"s << std::endl;
    std::cout << "Was send pushes: "s << pushes_num << std::endl;
    for (const auto& client : clients) {
        std::string s =
            std::format("Client {} received {} pushes", client->GetId(), client->GetReceivedPushes().size());
        std::cout << s << std::endl;
    }
    std::cout << "----------------------------------------------------------------------------\n" << std::endl;

    std::cout << "\nSource pushes:"s << std::endl;
    std::cout << pushes;
    std::cout << "----------------------------------------------------------------------------\n" << std::endl;

    std::cout << "Client 0 received pushes:"s << std::endl;
    std::cout << clients[0]->GetReceivedPushes() << std::endl;
}
