#include <memory>
#include <string>
#include <utility>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "dispatcher.h"
#include "exception.h"

class ExtraParamTest : public ::testing::Test {
public:
    wwa::json_rpc::dispatcher& dispatcher() noexcept { return *this->m_dispatcher; }

protected:
    void SetUp() override { this->m_dispatcher = std::make_unique<wwa::json_rpc::dispatcher>(); }

private:
    std::unique_ptr<wwa::json_rpc::dispatcher> m_dispatcher;
};

struct extra_data {
    std::string ip;
    nlohmann::json extra;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(extra_data, ip, extra);

struct extra_data_ip {
    std::string ip;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(extra_data_ip, ip);

using namespace nlohmann::json_literals;

TEST_F(ExtraParamTest, TestClosure)
{
    const auto request = nlohmann::json(
        {{"jsonrpc", "2.0"}, {"method", "test"}, {"params", {1, 2}}, {"id", 1}, {"auth", "secret"}, {"user", "admin"}}
    );

    const std::string expected_ip = "127.0.0.1";
    const auto expected_extra     = nlohmann::json({{"auth", request["auth"]}, {"user", request["user"]}});

    this->dispatcher().add_ex("test", [&expected_ip, &expected_extra, &request](const extra_data& extra, int a, int b) {
        EXPECT_EQ(extra.ip, expected_ip);
        EXPECT_EQ(extra.extra, expected_extra);
        EXPECT_EQ(a, request["params"][0]);
        EXPECT_EQ(b, request["params"][1]);
    });

    extra_data extra;
    extra.ip = expected_ip;

    const auto expected = nlohmann::json({{"jsonrpc", "2.0"}, {"id", 1}, {"result", nullptr}});
    const auto response = this->dispatcher().process_request(request, extra);
    EXPECT_EQ(response, expected);
}

TEST_F(ExtraParamTest, TestMethod)
{
    class helper {
    public:
        helper(nlohmann::json request, nlohmann::json expected_extra, std::string expected_ip)
            : m_request(std::move(request)), m_expected_extra(std::move(expected_extra)),
              m_expected_ip(std::move(expected_ip))
        {}

        void test(const extra_data& extra, int a, int b)
        {
            EXPECT_EQ(extra.ip, this->m_expected_ip);
            EXPECT_EQ(extra.extra, this->m_expected_extra);
            EXPECT_EQ(a, this->m_request["params"][0]);
            EXPECT_EQ(b, this->m_request["params"][1]);
        }

    private:
        nlohmann::json m_request;
        nlohmann::json m_expected_extra;
        std::string m_expected_ip;
    };

    const auto request = nlohmann::json(
        {{"jsonrpc", "2.0"}, {"method", "test"}, {"params", {1, 2}}, {"id", 1}, {"auth", "secret"}, {"user", "admin"}}
    );

    const std::string expected_ip = "127.0.0.1";
    const auto expected_extra     = nlohmann::json({{"auth", request["auth"]}, {"user", request["user"]}});

    helper h(request, expected_extra, expected_ip);

    this->dispatcher().add_ex("test", &helper::test, &h);

    extra_data extra;
    extra.ip = expected_ip;

    const auto expected = nlohmann::json({{"jsonrpc", "2.0"}, {"id", 1}, {"result", nullptr}});
    const auto response = this->dispatcher().process_request(request, extra);
    EXPECT_EQ(response, expected);
}

TEST_F(ExtraParamTest, TestClosureWithoutExtraFields)
{
    const auto request = nlohmann::json(
        {{"jsonrpc", "2.0"}, {"method", "test"}, {"params", {1, 2}}, {"id", 1}, {"auth", "secret"}, {"user", "admin"}}
    );

    const std::string expected_ip = "127.0.0.1";

    this->dispatcher().add_ex("test", [&expected_ip, &request](const extra_data_ip& extra, int a, int b) {
        EXPECT_EQ(extra.ip, expected_ip);
        EXPECT_EQ(a, request["params"][0]);
        EXPECT_EQ(b, request["params"][1]);
    });

    extra_data_ip extra;
    extra.ip = expected_ip;

    const auto expected = nlohmann::json({{"jsonrpc", "2.0"}, {"id", 1}, {"result", nullptr}});
    const auto response = this->dispatcher().process_request(request, extra);
    EXPECT_EQ(response, expected);
}

TEST_F(ExtraParamTest, TestClosureWithExtraJson)
{
    const auto request = nlohmann::json(
        {{"jsonrpc", "2.0"}, {"method", "test"}, {"params", {1, 2}}, {"id", 1}, {"auth", "secret"}, {"user", "admin"}}
    );

    const std::string expected_ip = "127.0.0.1";

    const auto expected_extra =
        nlohmann::json({{"ip", expected_ip}, {"extra", {{"auth", request["auth"]}, {"user", request["user"]}}}});

    this->dispatcher().add_ex("test", [&expected_extra, &request](const nlohmann::json& extra, int a, int b) {
        EXPECT_EQ(extra, expected_extra);
        EXPECT_EQ(a, request["params"][0]);
        EXPECT_EQ(b, request["params"][1]);
    });

    const auto extra    = nlohmann::json({{"ip", expected_ip}});
    const auto expected = nlohmann::json({{"jsonrpc", "2.0"}, {"id", 1}, {"result", nullptr}});
    const auto response = this->dispatcher().process_request(request, extra);
    EXPECT_EQ(response, expected);
}

TEST_F(ExtraParamTest, TestClosureWithExtraJsonNull)
{
    const auto request =
        nlohmann::json({{"jsonrpc", "2.0"}, {"method", "test"}, {"id", 1}, {"auth", "secret"}, {"user", "admin"}});

    this->dispatcher().add_ex("test", [](const nlohmann::json& extra) { EXPECT_TRUE(extra.is_null()); });

    const auto expected = nlohmann::json({{"jsonrpc", "2.0"}, {"id", 1}, {"result", nullptr}});
    const auto response = this->dispatcher().process_request(request, nlohmann::json());
    EXPECT_EQ(response, expected);
}

TEST_F(ExtraParamTest, TestException)
{
    this->dispatcher().add_ex("test", [](const extra_data_ip&, int, int) {
        FAIL() << "This method should not be called";
    });

    const auto request = nlohmann::json({{"jsonrpc", "2.0"}, {"method", "test"}, {"params", {1, 2}}, {"id", 1}});

    const auto expected = nlohmann::json(
        {{"jsonrpc", "2.0"},
         {"id", 1},
         {"error",
          {{"code", wwa::json_rpc::exception::INVALID_REQUEST},
           {"message", "[json.exception.out_of_range.403] key 'ip' not found"}}}}
    );

    const auto extra    = nlohmann::json({{"unexpected", "data"}});
    const auto response = this->dispatcher().process_request(request, extra);
    EXPECT_EQ(response, expected);
}
