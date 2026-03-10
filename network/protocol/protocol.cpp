#include "protocol.h"

#include <cstring>
#include <stdexcept>

// Кроссплатформенная поддержка сетевых функций
#if defined(_WIN32)
// Windows network API
#include <winsock2.h>
#else
// POSIX network API (Linux, macOS, etc.)
#include <arpa/inet.h>
#endif

namespace protocol {
namespace internal {
// Кодирует payload в frame: [length:LENGTH_PREFIX_SIZE][payload]
std::vector<uint8_t> EncodeFrame(std::span<const uint8_t> payload) {
    if (payload.size() > MAX_MESSAGE_SIZE) {
        throw std::runtime_error("Payload exceeds MAX_MESSAGE_SIZE!");
    }

    std::vector<uint8_t> frame(LENGTH_PREFIX_SIZE + payload.size());

    uint32_t len = htonl(static_cast<uint32_t>(payload.size()));
    std::memcpy(frame.data(), &len, sizeof(len));
    std::memcpy(frame.data() + LENGTH_PREFIX_SIZE, payload.data(), payload.size());

    return frame;
}

bool TryDecodeFrame(std::vector<uint8_t>& buffer, std::vector<uint8_t>& out) {
    if (buffer.size() < LENGTH_PREFIX_SIZE) {
        return false;
    }

    uint32_t len = 0;
    std::memcpy(&len, buffer.data(), sizeof(len));
    len = ntohl(len);

    if (len > MAX_MESSAGE_SIZE) {
        throw std::runtime_error("Frame length exceeds MAX_MESSAGE_SIZE");
    }

    size_t total_size = LENGTH_PREFIX_SIZE + len;
    if (buffer.size() < total_size) {
        return false;
    }

    out.assign(buffer.begin() + LENGTH_PREFIX_SIZE, buffer.begin() + total_size);
    buffer.erase(buffer.begin(), buffer.begin() + total_size);

    return true;
}
}  // namespace internal

std::vector<uint8_t> Message::EncodeMessage(const Message& message) {
    std::vector<uint8_t> data(MESSAGE_ID_SIZE + MESSAGE_TYPE_SIZE + message.payload.size());

    // id
    uint32_t net_id = htonl(message.id);
    std::memcpy(data.data(), &net_id, MESSAGE_ID_SIZE);
    // type
    data[MESSAGE_ID_SIZE] = static_cast<uint8_t>(message.type);
    // payload
    std::memcpy(data.data() + MESSAGE_HEADER_SIZE, message.payload.data(), message.payload.size());

    return data;
}

Message Message::DecodeMessage(std::span<const uint8_t> data) {
    if (data.size() < MESSAGE_HEADER_SIZE) {
        throw std::runtime_error("Message too short");
    }

    uint32_t net_id = 0;
    std::memcpy(&net_id, data.data(), MESSAGE_ID_SIZE);
    uint32_t id = ntohl(net_id);

    const uint8_t raw_type = data[MESSAGE_ID_SIZE];
    MessageType type;
    switch (raw_type) {
        case static_cast<uint8_t>(MessageType::Push):
        case static_cast<uint8_t>(MessageType::Accepted):
        case static_cast<uint8_t>(MessageType::Error):
            type = static_cast<MessageType>(raw_type);
            break;
        default:
            throw std::runtime_error("Unknown MessageType");
    }

    std::vector<uint8_t> payload;
    if (data.size() > MESSAGE_HEADER_SIZE) {
        payload.assign(data.begin() + MESSAGE_HEADER_SIZE, data.end());
    }

    if (type == MessageType::Push && payload.empty()) {
        throw std::runtime_error("Push message has empty payload");
    }

    return Message{id, type, std::move(payload)};
}

std::vector<uint8_t> ProtocolAdapter::SerializeToFrame(const Push& push,
                                                       const uint32_t message_id,
                                                       const MessageType message_type) const {
    std::vector<uint8_t> push_data = serializer_->Serialize(push);

    Message msg;
    msg.id = message_id;
    msg.type = message_type;
    msg.payload = std::move(push_data);

    std::vector<uint8_t> msg_data = Message::EncodeMessage(msg);

    return internal::EncodeFrame(msg_data);
}

std::optional<DecodedMessage> ProtocolAdapter::Feed(std::span<const uint8_t> chunk) {
    try {
        buffer_.insert(buffer_.end(), chunk.begin(), chunk.end());

        std::vector<uint8_t> msg_data;
        if (!internal::TryDecodeFrame(buffer_, msg_data)) {
            return std::nullopt;
        }

        Message msg = Message::DecodeMessage(msg_data);
        DecodedMessage decoded_msg;
        decoded_msg.id = msg.id;
        decoded_msg.type = msg.type;

        if (msg.type == MessageType::Push) {
            decoded_msg.push = serializer_->Deserialize(msg.payload);
        } else {
            decoded_msg.push = std::nullopt;
        }

        return decoded_msg;

    } catch (const std::exception&) {
        buffer_.clear();
        return std::nullopt;
    }
}
}  // namespace protocol
