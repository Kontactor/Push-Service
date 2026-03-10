#include "tests_utils.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <thread>

using namespace std::literals;
using namespace dispatcher;

namespace tests_utils {
namespace {
// return push with random filling
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
    return IPush{static_cast<PushType>(rand() % 3),
                 static_cast<PushPriority>(rand() % 3),
                 payloads[rand() % payloads.size()],
                 "admin"s};
}
}  // namespace

// Create deque of clients with size equal to num
std::deque<std::shared_ptr<IClient>> CreateClients(const uint32_t num) {
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

// =====================
// functions for testing
// =====================

// verify number of received pushes by clients
bool TestNumberOfReceivedPushes(const uint32_t clients_num, const uint32_t pushes_num) {
    // set clients (they will receive pushes) for server
    auto clients = CreateClients(clients_num);
    // create vector and fill with pushes
    std::vector<IPush> pushes = CreatePushes(pushes_num);
    {
        Server server(clients);
        // start send pushes
        for (auto& push : pushes) {
            server.PostPushNotification(std::make_shared<IPush>(push));
        }
    }

    for (const auto& client : clients) {
        if (client->GetReceivedPushes().size() != pushes_num) {
            return false;
        }
    }
    return true;
}

// verify order of received pushes by clients
bool TestOrderOfPushes(const uint32_t pushes_num) {
    // set clients (they will receive pushes) for server
    auto clients = CreateClients(1);
    // create vector and fill with pushes
    std::vector<IPush> pushes = CreatePushes(pushes_num);
    {
        Server server(clients);
        // start send pushes
        for (auto& push : pushes) {
            server.PostPushNotification(std::make_shared<IPush>(push));
        }
    }
    // sortingg source pushes to receive true expected order
    std::sort(pushes.begin(), pushes.end(), [](const IPush& lhs, const IPush& rhs) {
        return static_cast<int>(lhs.priority) > static_cast<int>(rhs.priority);
    });

    const auto pushes_ptrs = clients[0]->GetReceivedPushes();

    if (pushes_num == pushes_ptrs.size()) {
        return std::equal(pushes.begin(),
                          pushes.end(),
                          pushes_ptrs.begin(),
                          pushes_ptrs.end(),
                          [](const IPush& lhs, const IPush& rhs) {
                              return std::make_tuple(lhs.type, lhs.priority, lhs.payload, lhs.sender) ==
                                     std::make_tuple(rhs.type, rhs.priority, rhs.payload, rhs.sender);
                          });
    }
    return false;
}
}  // namespace tests_utils
