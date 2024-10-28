#include <string>
#include <tuple>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "base.h"

using namespace std::string_literals;
using namespace nlohmann::json_literals;

class ErrorHandlingTest : public BaseDispatcherTest,
                          public testing::WithParamInterface<std::tuple<std::string, nlohmann::json>> {};

TEST_P(ErrorHandlingTest, TestErrorHandling)
{
    const auto& [input, expected] = GetParam();
    const auto response           = this->dispatcher().parse_and_process_request(input);
    const auto actual             = nlohmann::json::parse(response);

    EXPECT_EQ(actual, expected);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(RequestParsingFromStandard, ErrorHandlingTest, testing::Values(
    // rpc call with invalid JSON
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz])"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::PARSE_ERROR },
                { "message", R"([json.exception.parse_error.101] parse error at line 1, column 40: syntax error while parsing object - invalid literal; last read: '"foobar, "p'; expected '}')" }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // rpc call with invalid Request object
    std::make_tuple(R"({"jsonrpc": "2.0", "method": 1, "params": "bar"})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                { "message", "[json.exception.type_error.302] type must be string, but is number" }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // rpc call with an empty Array
    std::make_tuple("[]"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                { "message", wwa::json_rpc::err_empty_batch }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // rpc call Batch, invalid JSON
    std::make_tuple(R"([{"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},{"jsonrpc": "2.0", "method"])"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::PARSE_ERROR },
                { "message", "[json.exception.parse_error.101] parse error at line 1, column 95: syntax error while parsing object separator - unexpected ']'; expected ':'" }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // rpc call with an invalid Batch (but not empty)
    std::make_tuple("[1]"s, nlohmann::json({
        {
            {
                "error", {
                    { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                    { "message", wwa::json_rpc::err_not_jsonrpc_2_0_request }
                }
            },
            { "id", nullptr },
            { "jsonrpc", "2.0" }
        }
    })),
    // rpc call with invalid Batch
    std::make_tuple("[1,2,3]"s, nlohmann::json({
        { { "error", { { "code", wwa::json_rpc::exception::INVALID_REQUEST }, { "message", wwa::json_rpc::err_not_jsonrpc_2_0_request } } }, { "id", nullptr }, { "jsonrpc", "2.0" } },
        { { "error", { { "code", wwa::json_rpc::exception::INVALID_REQUEST }, { "message", wwa::json_rpc::err_not_jsonrpc_2_0_request } } }, { "id", nullptr }, { "jsonrpc", "2.0" } },
        { { "error", { { "code", wwa::json_rpc::exception::INVALID_REQUEST }, { "message", wwa::json_rpc::err_not_jsonrpc_2_0_request } } }, { "id", nullptr }, { "jsonrpc", "2.0" } }
    }))
));

INSTANTIATE_TEST_SUITE_P(RequestParsing, ErrorHandlingTest, testing::Values(
    // Empty input
    std::make_tuple(""s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::PARSE_ERROR },
                { "message", "[json.exception.parse_error.101] parse error at line 1, column 1: attempting to parse an empty input; check that your input string or stream contains the expected JSON" }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // Empty method
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "", "id": 3})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                { "message", wwa::json_rpc::err_empty_method }
            }
        },
        { "id", 3 },
        { "jsonrpc", "2.0" }
    })),
    // Invalid JSON-RPC version
    std::make_tuple(R"({"jsonrpc": "12.0", "method": ""})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                { "message", wwa::json_rpc::err_not_jsonrpc_2_0_request }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // Missing field
    std::make_tuple(R"({"jsonrpc": "2.0"})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                { "message", "[json.exception.out_of_range.403] key 'method' not found" }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // Recursive batch
    std::make_tuple(R"([[]])"s, nlohmann::json({
        {
            {
                "error", {
                    { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                    { "message", wwa::json_rpc::err_not_jsonrpc_2_0_request }
                }
            },
            { "id", nullptr },
            { "jsonrpc", "2.0" }
        }
    })),
    // Bad ID
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "method", "id": true})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_REQUEST },
                { "message", wwa::json_rpc::err_bad_id_type }
            }
        },
        { "id", nullptr },
        { "jsonrpc", "2.0" }
    })),
    // Bad params
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "method", "id": 3, "params": 1})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_PARAMS },
                { "message", wwa::json_rpc::err_bad_params_type }
            }
        },
        { "id", 3 },
        { "jsonrpc", "2.0" }
    }))
));

INSTANTIATE_TEST_SUITE_P(MethodInvocation, ErrorHandlingTest, testing::Values(
    // rpc call of non-existent method
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "foobar", "id": "1"})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::METHOD_NOT_FOUND },
                { "message", wwa::json_rpc::err_method_not_found }
            }
        },
        { "id", "1" },
        { "jsonrpc", "2.0" }
    })),
    // Too many params (positional)
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "no_params", "id": 3, "params": [1]})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_PARAMS },
                { "message", wwa::json_rpc::err_invalid_params_passed_to_method }
            }
        },
        { "id", 3 },
        { "jsonrpc", "2.0" }
    })),
    // Too many params (named)
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "no_params", "id": 3, "params": {}})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_PARAMS },
                { "message", wwa::json_rpc::err_invalid_params_passed_to_method }
            }
        },
        { "id", 3 },
        { "jsonrpc", "2.0" }
    })),
    // Wrong params type
    std::make_tuple(R"({"jsonrpc": "2.0", "method": "subtract_p", "id": 3, "params": ["a", "b"]})"s, nlohmann::json({
        {
            "error", {
                { "code", wwa::json_rpc::exception::INVALID_PARAMS },
                { "message", "[json.exception.type_error.302] type must be number, but is string" }
            }
        },
        { "id", 3 },
        { "jsonrpc", "2.0" }
    }))
));
// clang-format on
