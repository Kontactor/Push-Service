#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "../push/push.h"

struct Push;

// Abstract interface for Push serialization
struct IPushSerializer {
    virtual ~IPushSerializer() = default;

    virtual std::vector<uint8_t> Serialize(const Push& push) const = 0;
    virtual Push Deserialize(std::span<const uint8_t> data) const = 0;
};
