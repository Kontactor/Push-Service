#include <cstring>
#include <gtest/gtest.h>

#include "protocol.h"
#include "push/push.h"
#include "serialization/binary_push_serializer.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

using namespace protocol;

class ProtocolTest : public ::testing::Test {
protected:
    ProtocolTest() : adapter(std::make_unique<BinaryPushSerializer>()) {}

    ProtocolAdapter adapter;
};

TEST_F(ProtocolTest, SerializeAndDeserializePush) {
    Push push;
    push.priority = PushPriority::High;
    push.category = PushCategory::Notification;
    push.source = PushSource::Source2;
    push.text = "Hello, this is a test push";

    auto frame = adapter.SerializeToFrame(push, 0, MessageType::Push);
    ASSERT_FALSE(frame.empty());

    auto result = adapter.Feed(frame);
    ASSERT_TRUE(result.has_value());

    const auto& decoded = *result;
    ASSERT_EQ(decoded.type, MessageType::Push);
    ASSERT_TRUE(decoded.push.has_value());

    const Push& decoded_push = *decoded.push;

    EXPECT_EQ(decoded_push.priority, push.priority);
    EXPECT_EQ(decoded_push.category, push.category);
    EXPECT_EQ(decoded_push.source, push.source);
    EXPECT_EQ(decoded_push.text, push.text);
}

TEST_F(ProtocolTest, EmptyTextPush) {
    Push push;
    push.priority = PushPriority::Normal;
    push.category = PushCategory::Warning;
    push.source = PushSource::Source1;
    push.text = "";

    auto frame = adapter.SerializeToFrame(push, 0, MessageType::Push);
    auto result = adapter.Feed(frame);

    ASSERT_TRUE(result.has_value());

    const auto& decoded = *result;
    ASSERT_EQ(decoded.type, MessageType::Push);
    ASSERT_TRUE(decoded.push.has_value());
    EXPECT_TRUE(decoded.push->text.empty());
}

TEST_F(ProtocolTest, LongTextPush) {
    Push push;
    push.priority = PushPriority::Critical;
    push.category = PushCategory::Error;
    push.source = PushSource::Source3;
    push.text = std::string(5000, 'A');

    auto frame = adapter.SerializeToFrame(push, 0, MessageType::Push);
    auto result = adapter.Feed(frame);

    ASSERT_TRUE(result.has_value());

    const auto& decoded = *result;
    EXPECT_EQ(decoded.id, 0);
    EXPECT_EQ(decoded.type, MessageType::Push);
    ASSERT_TRUE(decoded.push.has_value());
    EXPECT_EQ(decoded.push->text, push.text);
}

TEST(FrameTests, EncodeDecodeFrame) {
    std::vector<uint8_t> payload = {1, 2, 3, 4, 5};

    auto frame = internal::EncodeFrame(payload);
    ASSERT_EQ(frame.size(), protocol::LENGTH_PREFIX_SIZE + payload.size());

    std::vector<uint8_t> buffer = frame;
    std::vector<uint8_t> decoded;

    ASSERT_TRUE(internal::TryDecodeFrame(buffer, decoded));
    EXPECT_EQ(decoded, payload);
    EXPECT_TRUE(buffer.empty());
}

TEST(FrameTests, IncompleteFrame) {
    std::vector<uint8_t> buffer = {0x00, 0x00};  // меньше 4 байт

    std::vector<uint8_t> out;
    EXPECT_FALSE(internal::TryDecodeFrame(buffer, out));
}

TEST(FrameTests, OversizedFrame) {
    std::vector<uint8_t> buffer(protocol::LENGTH_PREFIX_SIZE);
    uint32_t len = protocol::MAX_MESSAGE_SIZE + 1;

    memcpy(buffer.data(), &len, sizeof(len));

    std::vector<uint8_t> out;
    EXPECT_THROW({ internal::TryDecodeFrame(buffer, out); }, std::runtime_error);
}

TEST(MessageTests, EncodeDecodeMessage) {
    Message msg;
    msg.id = 42;
    msg.type = MessageType::Push;
    msg.payload = {10, 20, 30};

    auto encoded = Message::EncodeMessage(msg);
    ASSERT_EQ(encoded.size(), MESSAGE_HEADER_SIZE + msg.payload.size());

    auto decoded = Message::DecodeMessage(encoded);
    EXPECT_EQ(decoded.id, msg.id);
    EXPECT_EQ(decoded.type, msg.type);
    EXPECT_EQ(decoded.payload, msg.payload);
}

TEST(MessageTests, DecodeTooShortMessage) {
    std::vector<uint8_t> data;

    EXPECT_THROW({ Message::DecodeMessage(data); }, std::runtime_error);
}

TEST(MessageTests, InvalidMessageType) {
    std::vector<uint8_t> data = {0xFF, 0x01, 0x02};

    EXPECT_THROW({ Message::DecodeMessage(data); }, std::runtime_error);
}

TEST_F(ProtocolTest, AcceptedType) {
    Message msg;
    msg.id = 42;
    msg.type = MessageType::Accepted;
    msg.payload = {1, 2, 3};

    auto msg_data = Message::EncodeMessage(msg);
    auto frame = internal::EncodeFrame(msg_data);

    auto result = adapter.Feed(frame);
    ASSERT_TRUE(result.has_value());

    const auto& decoded = *result;
    EXPECT_EQ(decoded.id, 42);
    EXPECT_EQ(decoded.type, MessageType::Accepted);
    EXPECT_FALSE(decoded.push.has_value());
}

TEST_F(ProtocolTest, GarbageData) {
    std::vector<uint8_t> garbage(100, 0xFF);

    auto result = adapter.Feed(garbage);
    EXPECT_FALSE(result.has_value());
}

TEST_F(ProtocolTest, FeedByteByByte) {
    Push original_push;
    original_push.priority = PushPriority::High;
    original_push.text = "Byte-by-byte test message";

    auto full_frame = adapter.SerializeToFrame(original_push, 42, MessageType::Push);
    ASSERT_FALSE(full_frame.empty());

    // Очищаем буфер перед тестом (важно!)
    ProtocolAdapter byte_adapter(std::make_unique<BinaryPushSerializer>());

    // Подаём по одному байту
    for (size_t i = 0; i < full_frame.size() - 1; ++i) {
        std::span<const uint8_t> chunk(&full_frame[i], 1);
        auto result = byte_adapter.Feed(chunk);
        EXPECT_FALSE(result.has_value()) << "Message appeared too early at byte " << i;
    }

    // Последний байт
    std::span<const uint8_t> last_chunk(&full_frame.back(), 1);
    auto final_result = byte_adapter.Feed(last_chunk);

    ASSERT_TRUE(final_result.has_value());
    EXPECT_EQ(final_result->id, 42);
    EXPECT_EQ(final_result->type, MessageType::Push);
    ASSERT_TRUE(final_result->push.has_value());
    EXPECT_EQ(final_result->push->text, original_push.text);
    EXPECT_EQ(final_result->push->priority, original_push.priority);
}
