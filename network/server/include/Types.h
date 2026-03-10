#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace server {

class ClientSession;

using SessionId = uint64_t;

using OnSessionConnectedCallback = std::function<void(SessionId, std::shared_ptr<ClientSession>)>;
using OnSessionDisconnectedCallback = std::function<void(SessionId)>;
using OnDataReceivedCallback = std::function<void(SessionId, const std::vector<uint8_t>&)>;

}  // namespace server
