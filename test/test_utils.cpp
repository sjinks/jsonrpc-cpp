#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "utils.h"

class SerializeResponseTest : public testing::TestWithParam<std::tuple<nlohmann::json, std::string>> {};
class IsErrorResponseTest : public testing::TestWithParam<std::tuple<nlohmann::json, bool>> {};

TEST_P(SerializeResponseTest, TestSerializeResponse)
{
    const auto& [response, expected] = GetParam();

    const auto actual = wwa::json_rpc::serialize_repsonse(response);

    EXPECT_EQ(actual, expected);
}

using namespace std::string_literals;
using namespace nlohmann::json_literals;

INSTANTIATE_TEST_SUITE_P(
    Serialization, SerializeResponseTest,
    testing::Values(
        std::make_tuple(nlohmann::json(nlohmann::json::value_t::discarded), ""s),
        std::make_tuple(nlohmann::json::object(), "{}"s)
    )
);

TEST_P(IsErrorResponseTest, TestIsErrorResponse)
{
    const auto& [response, expected] = GetParam();

    const auto actual = wwa::json_rpc::is_error_response(response);

    EXPECT_EQ(actual, expected);
}

INSTANTIATE_TEST_SUITE_P(
    Responses, IsErrorResponseTest,
    testing::Values(
        std::make_tuple(nlohmann::json::array(), false), std::make_tuple(nlohmann::json::object(), false),
        std::make_tuple(nlohmann::json{{"error", nullptr}}, false),
        std::make_tuple(nlohmann::json{{"error", {{"code", -1}, {"message", "error"}}}}, true)
    )
);

TEST(GetErrorCodeTest, TestGetErrorCode)
{
    const int expected = -10;
    const nlohmann::json response{{"error", {{"code", expected}, {"message", "error"}}}};

    const auto actual = wwa::json_rpc::get_error_code(response);

    EXPECT_EQ(actual, expected);
}

TEST(GetErrorMessageTest, TestGetErrorMessage)
{
    const std::string expected = "error";
    const nlohmann::json response{{"error", {{"code", -1}, {"message", expected}}}};
    const auto actual = wwa::json_rpc::get_error_message(response);

    EXPECT_EQ(actual, expected);
}
