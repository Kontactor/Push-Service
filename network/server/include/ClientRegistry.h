#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include "TcpSession.h"
#include "Types.h"

namespace server {

// TODO:
// Подумать про RAII (деструктор): закрывать/проверять сессии при уничтожении реестра
class ClientRegistry {
public:
    void Add(const std::shared_ptr<ClientSession>& session);
    void Remove(SessionId id);
    void ClearAll();

    size_t ActiveClientsCount() const;
    bool Contains(SessionId id) const;

    template <class Callback>
    void ForEachClient(Callback&& callback) const {
        for (const auto& [id, session] : sessions_) {
            callback(id, session);
        }
    }

private:
    using SessionsMap = std::unordered_map<SessionId, std::shared_ptr<ClientSession>>;

    SessionsMap sessions_;
    mutable std::shared_mutex mtx_;
};

}  // namespace server
