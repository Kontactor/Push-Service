#pragma once

#include <queue>

#include "client_lib.h"
#include "console_utils.h"

using PushPtr = std::shared_ptr<Push>;
using PushHistory = std::vector<PushPtr>;

// thread-safe queue for Push ptr objects
template <typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(value);
    }

    const T& front() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.front();
    }
    void pop() {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }

    void clear() {
        std::queue<T, std::deque<T>> temp_queue;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.swap(temp_queue);
        }
    }

private:
    std::queue<T, std::deque<T>> queue_;

    mutable std::mutex mtx_;
};

using PushQueue = ThreadSafeQueue<PushPtr>;
using MsgQueue = ThreadSafeQueue<console_ui::Message>;

class ClientEventHandler : public client_lib::IClientEventHandler {
public:
    explicit ClientEventHandler(std::shared_ptr<PushQueue> push_queue, std::shared_ptr<MsgQueue> msg_queue);
    // IClientEventHandler
    void onPushReceived(uint32_t, const Push&) override;
    void onAcceptPush(uint32_t) override {}
    void onDataSend(std::size_t) override {}
    void onConnected() override;
    void onDisconnected() override;
    void onReconnect() override;
    void onError(const std::string&) override;

    bool isConnected() const;

    size_t historySize() const;

    void clearHistory();

    const PushHistory& getHistory() const;

private:
    void disconnect();
    void updateFullHistory() const;

private:
    std::shared_ptr<PushQueue> push_queue_;
    std::shared_ptr<MsgQueue> msg_queue_;

    mutable PushHistory session_last_history_;
    mutable PushHistory session_full_history_;

    bool is_connected_ = false;
    mutable std::mutex mtx_;
};
