
#include <gtest/gtest.h>

#include "../src/client_lib.h"

using namespace client_lib;
using namespace std::literals;

TEST(ClientLibTest, clientTypeToString) {
    EXPECT_EQ(clientTypeToString(ClientRole::Admin), "admin"sv);
    EXPECT_EQ(clientTypeToString(ClientRole::User), "user"sv);
}

TEST(ClientLibTest, stringToClientType) {
    EXPECT_EQ(stringToClientType("admin"sv), ClientRole::Admin);
    EXPECT_EQ(stringToClientType("user"sv), ClientRole::User);
    EXPECT_FALSE(stringToClientType("unknown"sv));
    EXPECT_FALSE(stringToClientType("invalid"sv));
}

TEST(ClientLibTest, AuthorizeClientValidCredentials) {
    auto admin = authorizeClient(ADMIN_TEST_LOGIN, ADMIN_TEST_PASSWORD);
    ASSERT_TRUE(admin.has_value());
    EXPECT_EQ(admin->role, ClientRole::Admin);

    auto user = authorizeClient(USER_TEST_LOGIN, USER_TEST_PASSWORD);
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->role, ClientRole::User);
}

TEST(ClientLibTest, AuthorizeClientInvalidCredentials) {
    auto invalid1 = authorizeClient("wrong", "wrong");
    EXPECT_FALSE(invalid1.has_value());

    auto invalid2 = authorizeClient(ADMIN_TEST_LOGIN, "wrong");
    EXPECT_FALSE(invalid2.has_value());

    auto invalid3 = authorizeClient("", "");
    EXPECT_FALSE(invalid3.has_value());
}
