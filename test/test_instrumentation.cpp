#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "../src/dispatcher.h"

class mocked_dispatcher : public wwa::json_rpc::dispatcher {
public:
    MOCK_METHOD(void, on_request, (const nlohmann::json&), (override));
    MOCK_METHOD(void, on_method, (const std::string&, const nlohmann::json&), (override));
    MOCK_METHOD(void, on_request_processed, (const std::string&, int, const nlohmann::json&), (override));
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

    {
        const testing::InSequence s;
        EXPECT_CALL(dispatcher, on_request(nlohmann::json::object()));
        EXPECT_CALL(
            dispatcher,
            on_request_processed(std::string{}, wwa::json_rpc::exception::INVALID_REQUEST, nlohmann::json::object())
        );
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

    {
        const testing::InSequence s;
        EXPECT_CALL(dispatcher, on_request(extra_data));
        EXPECT_CALL(dispatcher, on_method("add", expected_data_1));
        EXPECT_CALL(dispatcher, on_request_processed("add", 0, expected_data_1));

        EXPECT_CALL(dispatcher, on_request(extra_data));
        EXPECT_CALL(dispatcher, on_method("subtract", expected_data_2));
        EXPECT_CALL(dispatcher, on_request_processed("subtract", 0, expected_data_2));

        EXPECT_CALL(dispatcher, on_request(extra_data));
        EXPECT_CALL(dispatcher, on_method("add", expected_data_3));
        EXPECT_CALL(dispatcher, on_request_processed("add", wwa::json_rpc::exception::INVALID_PARAMS, expected_data_3));

        EXPECT_CALL(dispatcher, on_request(extra_data));
        EXPECT_CALL(
            dispatcher, on_request_processed("bad", wwa::json_rpc::exception::METHOD_NOT_FOUND, expected_data_4)
        );

        EXPECT_CALL(dispatcher, on_request(extra_data));
        EXPECT_CALL(dispatcher, on_request_processed("", wwa::json_rpc::exception::INVALID_REQUEST, expected_data_5));
    }

    dispatcher.process_request(input, extra_data);
}
