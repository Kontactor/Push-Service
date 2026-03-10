#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "push/push.h"
#include "serialization/push_serializer.h"

namespace protocol {
constexpr size_t LENGTH_PREFIX_SIZE = 4;                 // 4 байта
constexpr uint32_t MAX_MESSAGE_SIZE = 10 * 1024 * 1024;  // 10MB максимум

enum class MessageType : uint8_t { Push = 1, Accepted = 2, Error = 3 };

struct Message {
    uint32_t id;
    MessageType type;
    std::vector<uint8_t> payload;

    static std::vector<uint8_t> EncodeMessage(const Message& message);
    static Message DecodeMessage(std::span<const uint8_t> data);
};

constexpr size_t MESSAGE_ID_SIZE = sizeof(Message::id);
constexpr size_t MESSAGE_TYPE_SIZE = sizeof(Message::type);
constexpr size_t MESSAGE_HEADER_SIZE = MESSAGE_ID_SIZE + MESSAGE_TYPE_SIZE;

// Кодирует payload в frame: [length:LENGTH_PREFIX_SIZE][payload]
namespace internal {
std::vector<uint8_t> EncodeFrame(std::span<const uint8_t> payload);
bool TryDecodeFrame(std::vector<uint8_t>& buffer, std::vector<uint8_t>& out);
}  // namespace internal

struct DecodedMessage {
    uint32_t id;
    MessageType type;
    std::optional<Push> push;
};

class ProtocolAdapter {
public:
    explicit ProtocolAdapter(std::unique_ptr<IPushSerializer> serializer) : serializer_(std::move(serializer)) {}

    std::vector<uint8_t> SerializeToFrame(const Push& push,
                                          const uint32_t message_id,
                                          const MessageType message_type) const;
    std::optional<DecodedMessage> Feed(std::span<const uint8_t> chunk);

private:
    std::vector<uint8_t> buffer_;
    std::unique_ptr<IPushSerializer> serializer_;
};
}  // namespace protocol
