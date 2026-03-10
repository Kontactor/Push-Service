#include "push_generator.h"

#include <chrono>

PushGenerator::PushGenerator(client::TcpClient& client) : client_(client) {}

PushGenerator::~PushGenerator() {
    Stop();
}

void PushGenerator::Start() {
    running_ = true;
    worker_ = std::thread(&PushGenerator::Run, this);
}

void PushGenerator::Stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

void PushGenerator::Run() {
    while (running_) {
        Push push = GeneratePush();
        auto bytes = serializer_.Serialize(push);

        client_.asyncSend(bytes);

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
