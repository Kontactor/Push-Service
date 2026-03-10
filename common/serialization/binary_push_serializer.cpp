#include "binary_push_serializer.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <stdexcept>

// to ensure cross - platform compatibility
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

std::vector<uint8_t> BinaryPushSerializer::Serialize(const Push& push) const {
    std::vector<std::byte> buffer;
    buffer.reserve(3 + sizeof(uint32_t) + push.text.size());

    // enums — 1 byte each
    buffer.push_back(static_cast<std::byte>(push.priority));
    buffer.push_back(static_cast<std::byte>(push.category));
    buffer.push_back(static_cast<std::byte>(push.source));

    // message text length (network byte order convert)
    uint32_t text_size = static_cast<uint32_t>(push.text.size());
    text_size = htonl(text_size);

    auto size_bytes = std::bit_cast<std::array<std::byte, 4>>(text_size);
    buffer.insert(buffer.end(), size_bytes.begin(), size_bytes.end());

    const auto* text_bytes = reinterpret_cast<const std::byte*>(push.text.data());
    buffer.insert(buffer.end(), text_bytes, text_bytes + push.text.size());

    std::vector<uint8_t> result(buffer.size());
    std::memcpy(result.data(), buffer.data(), buffer.size());

    return result;
}

Push BinaryPushSerializer::Deserialize(std::span<const uint8_t> data) const {
    // 3 enums + 4 bytes message text length
    if (data.size() < 3 + sizeof(uint32_t)) {
        throw std::runtime_error("Invalid Push packet");
    }

    size_t offset = 0;

    Push push;
    // push.priority = static_cast<PushPriority>(data[offset++]);
    // push.category = static_cast<PushCategory>(data[offset++]);
    // push.source = static_cast<PushSource>(data[offset++]);
    auto priority = static_cast<PushPriority>(data[offset++]);
    if (!IsValid(priority)) {
        throw std::runtime_error("Invalid PushPriority");
    }

    auto category = static_cast<PushCategory>(data[offset++]);
    if (!IsValid(category)) {
        throw std::runtime_error("Invalid PushCategory");
    }

    auto source = static_cast<PushSource>(data[offset++]);
    if (!IsValid(source)) {
        throw std::runtime_error("Invalid PushSource");
    }

    push.priority = priority;
    push.category = category;
    push.source = source;

    // message text length
    std::array<std::byte, 4> size_bytes{};
    std::memcpy(size_bytes.data(), data.data() + offset, sizeof(uint32_t));

    offset += sizeof(uint32_t);

    uint32_t text_size = std::bit_cast<uint32_t>(size_bytes);
    // message text length (network byte order convert)
    text_size = ntohl(text_size);

    if (offset + text_size > data.size()) {
        throw std::runtime_error("Invalid Push text size");
    }

    push.text.assign(reinterpret_cast<const char*>(data.data() + offset), text_size);

    return push;
}
