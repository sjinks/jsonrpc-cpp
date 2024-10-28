#include <string_view>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "../src/exception.h"

using namespace std::string_view_literals;

TEST(ExceptionTest, TestExceptionConstructor)
{
    const auto code    = wwa::json_rpc::exception::PARSE_ERROR;
    const auto message = "Parse error"sv;

    const wwa::json_rpc::exception e(code, message);

    EXPECT_EQ(e.code(), code);
    EXPECT_EQ(e.message(), message);
    EXPECT_STRCASEEQ(e.what(), message.data());
    EXPECT_EQ(e.data().is_null(), true);
}

TEST(ExceptionTest, TestExceptionConstructorWithData)
{
    const auto code    = wwa::json_rpc::exception::PARSE_ERROR;
    const auto message = "Parse error"sv;
    const auto data    = 123U;

    const wwa::json_rpc::exception e(code, message, data);

    EXPECT_EQ(e.code(), code);
    EXPECT_EQ(e.message(), message);
    EXPECT_STRCASEEQ(e.what(), message.data());
    EXPECT_EQ(e.data(), nlohmann::json(data));
}

TEST(ExceptionTest, TestExceptionToJson)
{
    const auto code     = wwa::json_rpc::exception::PARSE_ERROR;
    const auto message  = "Parse error"sv;
    const auto data     = 123U;
    const auto expected = nlohmann::json{{"code", code}, {"message", message}, {"data", data}};

    const wwa::json_rpc::exception e(code, message, data);
    const auto j = e.to_json();

    EXPECT_EQ(j, expected);
}

TEST(ExceptionTest, TestExceptionToJsonNoData)
{
    const auto code     = wwa::json_rpc::exception::PARSE_ERROR;
    const auto message  = "Parse error"sv;
    const auto expected = nlohmann::json{{"code", code}, {"message", message}};

    const wwa::json_rpc::exception e(code, message);
    const auto j = e.to_json();

    EXPECT_EQ(j, expected);
}
