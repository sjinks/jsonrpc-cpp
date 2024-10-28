#include <tuple>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "base.h"

using namespace nlohmann::json_literals;

class MethodInvocationTest : public BaseDispatcherTest,
                             public testing::WithParamInterface<std::tuple<nlohmann::json, nlohmann::json>> {};

TEST_P(MethodInvocationTest, TestMethodCalls)
{
    const auto& [input, expected] = GetParam();
    const auto response           = this->dispatcher().process_request(input);
    const auto actual             = nlohmann::json::parse(response);

    EXPECT_EQ(actual, expected);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(MethodInvocation, MethodInvocationTest, testing::Values(
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "subtract", "params": {"minuend": 42, "subtrahend": 23}, "id": 3})"_json, R"({"jsonrpc":"2.0","result":19,"id":3})"_json),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "s_subtract", "params": {"minuend": 40, "subtrahend": 20}, "id": 3})"_json, R"({"jsonrpc":"2.0","result":20,"id":3})"_json),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "subtract_p", "params": [9, 2], "id": 3})"_json, R"({"jsonrpc":"2.0","result":7,"id":3})"_json),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "s_subtract_p", "params": [1, 1], "id": 3})"_json, R"({"jsonrpc":"2.0","result":0,"id":3})"_json),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "no_params", "id": 3})"_json, R"({"jsonrpc":"2.0","result":24,"id":3})"_json),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "no_params", "params": [], "id": 3})"_json, R"({"jsonrpc":"2.0","result":24,"id":3})"_json),
    std::make_tuple(
        R"([
            {"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
            {"jsonrpc": "2.0", "method": "notify_hello", "params": [7]},
            {"jsonrpc": "2.0", "method": "subtract_p", "params": [42,23], "id": "2"},
            {"foo": "boo"},
            {"jsonrpc": "2.0", "method": "foo.get", "params": {"name": "myself"}, "id": "5"},
            {"jsonrpc": "2.0", "method": "get_data", "id": "9"} 
        ])"_json,
        R"([
            {"jsonrpc":"2.0","result":7,"id":"1"},
            {"jsonrpc":"2.0","result":19,"id":"2"},
            {"jsonrpc":"2.0","error":{"code":-32600,"message":"[json.exception.out_of_range.403] key 'jsonrpc' not found"},"id":null},
            {"jsonrpc":"2.0","error":{"code":-32601,"message":"Method not found"},"id":"5"},
            {"jsonrpc":"2.0","result":["hello",5],"id":"9"}
        ])"_json
    ),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "throwing", "id": 1})"_json, R"({"jsonrpc":"2.0","error":{"code":-32603,"message":"test"},"id":1})"_json),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "sumv", "params": [1,2,4], "id": "1"})"_json, R"({"jsonrpc":"2.0","result":7,"id":"1"})"_json),
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "s_sumv", "params": [1,2,4], "id": "1"})"_json, R"({"jsonrpc":"2.0","result":7,"id":"1"})"_json)
));
// clang-format on
