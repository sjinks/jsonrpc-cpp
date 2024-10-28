#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "../src/dispatcher.h"

class mocked_dispatcher : public wwa::json_rpc::dispatcher {
public:
    MOCK_METHOD(void, on_request, (), (override));
    MOCK_METHOD(void, on_method, (const std::string&), (override));
    MOCK_METHOD(void, on_request_processed, (const std::string&, int), (override));
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
        EXPECT_CALL(dispatcher, on_request());
        EXPECT_CALL(dispatcher, on_request_processed(std::string{}, wwa::json_rpc::exception::INVALID_REQUEST));
    }

    dispatcher.process_request(input);
}

TEST_F(InstrumentationTest, BatchRequest)
{
    const auto input = R"([
        {"jsonrpc": "2.0", "method": "add", "params": [1, 2], "id": 1},
        {"jsonrpc": "2.0", "method": "subtract", "params": [2, 1], "id": 2},
        {"jsonrpc": "2.0", "method": "add", "params": ["2", "3"], "id": 3},
        {"jsonrpc": "2.0", "method": "bad", "id": 4}
    ])"_json;
    auto& dispatcher = this->dispatcher();

    {
        const testing::InSequence s;
        EXPECT_CALL(dispatcher, on_request());
        EXPECT_CALL(dispatcher, on_method("add"));
        EXPECT_CALL(dispatcher, on_request_processed("add", 0));

        EXPECT_CALL(dispatcher, on_request());
        EXPECT_CALL(dispatcher, on_method("subtract"));
        EXPECT_CALL(dispatcher, on_request_processed("subtract", 0));

        EXPECT_CALL(dispatcher, on_request());
        EXPECT_CALL(dispatcher, on_method("add"));
        EXPECT_CALL(dispatcher, on_request_processed("add", wwa::json_rpc::exception::INVALID_PARAMS));

        EXPECT_CALL(dispatcher, on_request());
        EXPECT_CALL(dispatcher, on_request_processed("bad", wwa::json_rpc::exception::METHOD_NOT_FOUND));
    }

    dispatcher.process_request(input);
}
