#include <string>

#include <gtest/gtest.h>

#include "base.h"

using namespace std::string_literals;

class NotificationsTest : public BaseDispatcherTest,
                          public testing::WithParamInterface<std::string> {};

TEST_P(NotificationsTest, TestNotifications)
{
    const auto& input = GetParam();
    const auto actual = this->dispatcher().process_request(nlohmann::json::parse(input));

    EXPECT_TRUE(actual.is_discarded());
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(Notifications, NotificationsTest, testing::Values(
    R"({"jsonrpc": "2.0", "method": "notification"})"s,
    R"({"jsonrpc": "2.0", "method": "s_notification"})"s,
    // rpc call Batch (all notifications)
    R"([{"jsonrpc": "2.0", "method": "notification"},{"jsonrpc": "2.0", "method": "s_notification"}])"s
));
// clang-format on
