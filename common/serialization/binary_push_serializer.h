#pragma once

#include "push_serializer.h"

class BinaryPushSerializer : public IPushSerializer {
public:
    std::vector<uint8_t> Serialize(const Push& push) const;
    Push Deserialize(std::span<const uint8_t> data) const;
};
