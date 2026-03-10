#include "../include/dispatcher.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <format>
#include <iostream>
#include <mutex>
#include <optional>
#include <set>
#include <thread>

namespace dispatcher {

using namespace std::literals;
using push_ptr = std::shared_ptr<IPush>;

// imitation of logger class
class Logger {
public:
    explicit Logger(std::ostream& out = std::cout) : out_(out) {}

    void Log(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        std::string res = std::format("{} : {}\n", GetCurrentTime(), msg);
        out_ << res << std::flush;
    }

private:
    std::string GetCurrentTime() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::mutex mtx_;
    std::ostream& out_;
};

// function return thread id for logging
size_t ThreadIdToSizet(const std::thread::id id) {
    return std::hash<std::thread::id>{}(id);
}

// class container analog priority_queue
// firstly return elements with max priority
class PushQueue {
public:
    explicit PushQueue(const uint32_t priority_count) : priority_queues_(priority_count, std::deque<push_ptr>{}) {}
    // add push_ptr to queue
    void push(push_ptr push) {
        const int deq_index = static_cast<int>(push->priority);
        priority_queues_[deq_index].emplace_back(std::move(push));
        if (current_deq_.has_value() && *current_deq_ > deq_index) {
            return;
        }
        current_deq_ = deq_index;
    }

    // return front element with max priority
    push_ptr front() {
        push_ptr push = priority_queues_[*current_deq_].front();
        return push;
    }

    // remove front element from queue
    void pop() {
        priority_queues_[*current_deq_].pop_front();
        UpdateCurrentDeq();
    }

    bool empty() const {
        return !current_deq_.has_value();
    }

private:
    std::vector<std::deque<push_ptr>> priority_queues_;
    // number of not empty deque with maximum priority
    std::optional<int> current_deq_{};

    // update current_deq_ and writes to it the not empty deque number
    // with the highest priority or nullopt
    void UpdateCurrentDeq() {
        if (!priority_queues_[*current_deq_].empty()) {
            return;
        }

        size_t i = *current_deq_;
        while (i > 0) {
            --i;
            if (!priority_queues_[i].empty()) {
                current_deq_ = i;
                return;
            }
        }
        current_deq_ = std::nullopt;
    }
};

// =====================
// Dispatcher implementation
// =====================
class Dispatcher : public IDispatcher {
public:
    Dispatcher(const std::deque<std::shared_ptr<IClient>>& clients) :
        push_queue_(priority_count_), stopped_(false), clients_(clients),
        processing_thread_(&Dispatcher::Process, this) {
        // logging
        std::string log = std::format("Dispatcher start in two threads: {}, {}",
                                      ThreadIdToSizet(std::this_thread::get_id()),
                                      ThreadIdToSizet(processing_thread_.get_id()));
        logger_.Log(std::move(log));
    }

    ~Dispatcher() override;

    void Post(std::shared_ptr<IPush> push) override;

    void Stop() override;

private:
    void Process();

    bool Send(const push_ptr& push);
    // put the push in the queue if not dispatcher not stopped
    void EnqueueOrNot(const std::shared_ptr<IPush>& push);
    // return smart pointer to push with max priority and remove it from the pushes queue
    std::optional<push_ptr> GetPushFromQueue();

private:
    // number of priority of pushes
    const uint32_t priority_count_ = 3;
    PushQueue push_queue_;
    std::mutex queue_mutex_;
    std::condition_variable push_queue_condition_;
    std::atomic<bool> stopped_;

    // TODO: replace the link to the queue with clients imitation with
    // a link to the queue of real cliants or sessions
    const ClientsQueue& clients_;
    // TODO: replace the link to the logger imitation with
    // a link to the real logger
    Logger logger_;

    std::jthread processing_thread_;
};

// =====================
// Dispatcher methods
// =====================

Dispatcher::~Dispatcher() {
    if (!stopped_) {
        Stop();
    }
    logger_.Log("Dispatcher stopped"s);
}

void Dispatcher::Post(std::shared_ptr<IPush> push) {
    // TODO: Handle nullptr explicitly (logging / exception / error code).
    if (!push) {
        logger_.Log("ERROR! Push is null"s);
        return;
    }
    EnqueueOrNot(push);
}

void Dispatcher::Stop() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        // logging
        std::string log =
            std::format("Dispatcher thread {} received command \"STOP\"", ThreadIdToSizet(std::this_thread::get_id()));
        logger_.Log(std::move(log));

        stopped_ = true;

        push_queue_condition_.notify_one();
    }
}

void Dispatcher::Process() {
    // logging
    size_t id = ThreadIdToSizet(std::this_thread::get_id());
    std::string log = std::format("Dispatcher thread {} start processing", id);
    logger_.Log(std::move(log));

    for (auto push = GetPushFromQueue(); push.has_value(); push = GetPushFromQueue()) {
        if (Send(push.value())) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            push_queue_.pop();
        }
    }

    // logging
    log = std::format("Dispatcher thread {} stoped processing", id);
    logger_.Log(std::move(log));
}

bool Dispatcher::Send(const std::shared_ptr<IPush>& push) {
    for (auto client : clients_) {
        client->SendPush(push);
    }
    // TODO: add send error handling
    return true;
}

// put the push in the queue
void Dispatcher::EnqueueOrNot(const std::shared_ptr<IPush>& push) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        if (stopped_) {
            return;  // after Stop new push don't accept
        }

        push_queue_.push(push);

        push_queue_condition_.notify_one();
    }
}

// return smart pointer to push with max priority from the pushes queue
std::optional<push_ptr> Dispatcher::GetPushFromQueue() {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    if (push_queue_.empty() && !stopped_) {
        push_queue_condition_.wait(lock, [this] {
            return stopped_ || !push_queue_.empty();
        });
    }

    if (stopped_ && push_queue_.empty()) {
        return std::nullopt;
    }

    return push_queue_.front();
}

std::unique_ptr<IDispatcher> makeDispatcher(const ClientsQueue& clients) {
    return std::make_unique<Dispatcher>(clients);
};
}  // namespace dispatcher
