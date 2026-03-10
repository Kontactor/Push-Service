#pragma once

#include <memory>
#include <optional>

#include "common.h"

namespace push_storage {
// an interface that provides access to a database
class IPushStorage {
public:
    virtual bool OpenDatabase() = 0;

    virtual bool InsertPush(int sender_id, const IPush& push) = 0;

    virtual std::optional<std::string> GetLastError() const = 0;

    virtual ~IPushStorage() = default;
};

std::unique_ptr<IPushStorage> makeStorage(const std::string& db_name);

}  // namespace push_storage
