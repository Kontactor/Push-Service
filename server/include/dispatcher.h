#pragma once

#include <memory>
#include <queue>

#include "common.h"

namespace dispatcher {

using ClientsQueue = std::deque<std::shared_ptr<IClient>>;

// dispatcher interface for sending push messages
class IDispatcher {
public:
    virtual ~IDispatcher() = default;
    virtual void Post(std::shared_ptr<IPush> push) = 0;
    virtual void Stop() = 0;
};

std::unique_ptr<IDispatcher> makeDispatcher(const ClientsQueue& clients);
}  // namespace dispatcher
