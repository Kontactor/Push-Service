#include <cassert>
#include <iostream>

#include "../push/push.h"
#include "../serialization/binary_push_serializer.h"

static void TestBasicRoundtrip() {
    Push original;
    original.priority = PushPriority::High;
    original.category = PushCategory::Warning;
    original.source = PushSource::Source1;
    original.text = "Hello, Push!";

    BinaryPushSerializer serializer;

    auto data = serializer.Serialize(original);
    Push restored = serializer.Deserialize(data);

    assert(original.priority == restored.priority);
    assert(original.category == restored.category);
    assert(original.source == restored.source);
    assert(original.text == restored.text);

    std::cout << "[OK] Basic roundtrip\n";
}

static void TestEmptyText() {
    Push original;
    original.priority = PushPriority::Normal;
    original.category = PushCategory::Notification;
    original.source = PushSource::Source2;
    original.text = "";

    BinaryPushSerializer serializer;

    auto data = serializer.Serialize(original);
    Push restored = serializer.Deserialize(data);

    assert(restored.text.empty());
    assert(original.priority == restored.priority);

    std::cout << "[OK] Empty text\n";
}

static void TestLongText() {
    Push original;
    original.priority = PushPriority::Critical;
    original.category = PushCategory::Error;
    original.source = PushSource::Source1;
    original.text = std::string(10'000, 'A');

    BinaryPushSerializer serializer;

    auto data = serializer.Serialize(original);
    Push restored = serializer.Deserialize(data);

    assert(original.text == restored.text);

    std::cout << "[OK] Long text (10k chars)\n";
}

static void TestInvalidPacket() {
    BinaryPushSerializer serializer;

    try {
        std::vector<uint8_t> garbage = {1, 2, 3};
        serializer.Deserialize(garbage);

        assert(false && "Exception expected");
    } catch (const std::exception&) {
        std::cout << "[OK] Invalid packet rejected\n";
    }
}

int main() {
    TestBasicRoundtrip();
    TestEmptyText();
    TestLongText();
    TestInvalidPacket();

    std::cout << "\nAll push serialization tests passed\n";
    return 0;
}
