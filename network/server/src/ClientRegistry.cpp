#include "ClientRegistry.h"

namespace server {

void ClientRegistry::Add(const std::shared_ptr<ClientSession>& session) {
    if (!session) {
        return;
    }
    std::lock_guard<std::shared_mutex> lock(mtx_);
    sessions_[session->getId()] = session;
}

void ClientRegistry::Remove(SessionId id) {
    std::lock_guard<std::shared_mutex> lock(mtx_);
    sessions_.erase(id);
}

void ClientRegistry::ClearAll() {
    SessionsMap sessions;
    {
        std::lock_guard<std::shared_mutex> lock(mtx_);
        sessions.swap(sessions_);
    }
    for (const auto& [id, session] : sessions) {
        if (session) {
            session->close();
        }
    }
}

size_t ClientRegistry::ActiveClientsCount() const {
    std::shared_lock lock(mtx_);
    return sessions_.size();
}

bool ClientRegistry::Contains(SessionId id) const {
    std::shared_lock lock(mtx_);
    return sessions_.contains(id);
}

}  // namespace server
