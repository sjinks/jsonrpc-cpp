#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "dispatcher.h"
#include "exception.h"

class mocked_dispatcher : public wwa::json_rpc::dispatcher {
public:
    MOCK_METHOD(void, on_request, (const nlohmann::json&, const nlohmann::json&), (override));
    MOCK_METHOD(void, on_method, (const std::string&, const nlohmann::json&), (override));
    MOCK_METHOD(
        void, on_request_processed, (const std::string&, const nlohmann::json&, const nlohmann::json&), (override)
    );
};

class InstrumentationTest : public ::testing::Test {
public:
    InstrumentationTest()
    {
        this->m_dispatcher.add("add", [](int a, int b) { return a + b; });
        this->m_dispatcher.add("subtract", [](int a, int b) { return a - b; });
    }

    mocked_dispatcher& dispatcher() noexcept { return this->m_dispatcher; }

private:
    testing::StrictMock<mocked_dispatcher> m_dispatcher;
};

using namespace nlohmann::json_literals;

TEST_F(InstrumentationTest, BadRequest)
{
    const auto input = R"([])"_json;
    auto& dispatcher = this->dispatcher();

    const nlohmann::json expected_response{
        {"jsonrpc", "2.0"},
        {"error", {{"code", wwa::json_rpc::exception::INVALID_REQUEST}, {"message", wwa::json_rpc::err_empty_batch}}},
        {"id", nullptr}
    };

    {
        const testing::InSequence s;
        EXPECT_CALL(dispatcher, on_request(input, nlohmann::json::object()));
        EXPECT_CALL(dispatcher, on_request_processed(std::string{}, expected_response, nlohmann::json::object()));
    }

    dispatcher.process_request(input);
}

TEST_F(InstrumentationTest, BatchRequest)
{
    const auto input = R"([
        {"jsonrpc": "2.0", "method": "add", "params": [1, 2], "id": 1, "extra": "extra1" },
        {"jsonrpc": "2.0", "method": "subtract", "params": [2, 1], "id": 2, "extra": "extra2"},
        {"jsonrpc": "2.0", "method": "add", "params": ["2", "3"], "id": 3, "extra": "extra3"},
        {"jsonrpc": "2.0", "method": "bad", "id": 4, "extra": "extra4"},
        {"extra": "extra5"}
    ])"_json;
    auto& dispatcher = this->dispatcher();

    const auto extra_data       = R"({"ip": "127.0.0.1"})"_json;
    const auto expected_data_1  = nlohmann::json({{"ip", "127.0.0.1"}, {"extra", {{"extra", "extra1"}}}});
    const auto expected_data_2  = nlohmann::json({{"ip", "127.0.0.1"}, {"extra", {{"extra", "extra2"}}}});
    const auto expected_data_3  = nlohmann::json({{"ip", "127.0.0.1"}, {"extra", {{"extra", "extra3"}}}});
    const auto expected_data_4  = nlohmann::json({{"ip", "127.0.0.1"}, {"extra", {{"extra", "extra4"}}}});
    const auto& expected_data_5 = extra_data;

    const auto expected_response_1 = R"({"jsonrpc": "2.0", "id": 1, "result": 3})"_json;
    const auto expected_response_2 = R"({"jsonrpc": "2.0", "id": 2, "result": 1})"_json;
    const auto expected_response_3 = nlohmann::json(
        {{"error",
          {{"code", wwa::json_rpc::exception::INVALID_PARAMS},
           {"message", "[json.exception.type_error.302] type must be number, but is string"}}},
         {"id", 3},
         {"jsonrpc", "2.0"}}
    );
    const auto expected_response_4 = nlohmann::json(
        {{"error",
          {{"code", wwa::json_rpc::exception::METHOD_NOT_FOUND}, {"message", wwa::json_rpc::err_method_not_found}}},
         {"id", 4},
         {"jsonrpc", "2.0"}}
    );
    const auto expected_response_5 = nlohmann::json(
        {{"error",
          {{"code", wwa::json_rpc::exception::INVALID_REQUEST},
           {"message", "[json.exception.out_of_range.403] key 'jsonrpc' not found"}}},
         {"id", nullptr},
         {"jsonrpc", "2.0"}}
    );

    {
        const testing::InSequence s;
        EXPECT_CALL(dispatcher, on_request(input.at(0), extra_data));
        EXPECT_CALL(dispatcher, on_method("add", expected_data_1));
        EXPECT_CALL(dispatcher, on_request_processed("add", expected_response_1, expected_data_1));

        EXPECT_CALL(dispatcher, on_request(input.at(1), extra_data));
        EXPECT_CALL(dispatcher, on_method("subtract", expected_data_2));
        EXPECT_CALL(dispatcher, on_request_processed("subtract", expected_response_2, expected_data_2));

        EXPECT_CALL(dispatcher, on_request(input.at(2), extra_data));
        EXPECT_CALL(dispatcher, on_method("add", expected_data_3));
        EXPECT_CALL(dispatcher, on_request_processed("add", expected_response_3, expected_data_3));

        EXPECT_CALL(dispatcher, on_request(input.at(3), extra_data));
        EXPECT_CALL(dispatcher, on_request_processed("bad", expected_response_4, expected_data_4));

        EXPECT_CALL(dispatcher, on_request(input.at(4), extra_data));
        EXPECT_CALL(dispatcher, on_request_processed("", expected_response_5, expected_data_5));
    }

    dispatcher.process_request(input, extra_data);
}
