#include <gtest/gtest.h>

#include "tests_utils.h"

using namespace tests_utils;

TEST(PushDispatcherTest, AllPushIsDeliveredToClient) {
    ASSERT_EQ(TestNumberOfReceivedPushes(4, 10), true);
}

TEST(PushDispatcherTest, TrueOrderOfSendingPushes) {
    ASSERT_EQ(TestOrderOfPushes(10), true);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}
