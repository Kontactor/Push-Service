#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "TcpClient.h"
#include "binary_push_serializer.h"

class PushGenerator {
public:
    explicit PushGenerator(client::TcpClient& client);
    virtual ~PushGenerator();

    void Start();
    void Stop();

protected:
    virtual Push GeneratePush() = 0;

protected:
    client::TcpClient& client_;
    BinaryPushSerializer serializer_;

private:
    void Run();

    std::atomic<bool> running_{false};
    std::thread worker_;
};
