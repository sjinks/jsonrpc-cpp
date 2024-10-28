#ifndef DF466239_E1AA_4975_87F2_5A43B9623F91
#define DF466239_E1AA_4975_87F2_5A43B9623F91

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "../src/dispatcher.h"

struct subtract_params {
    int minuend;
    int subtrahend;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(subtract_params, minuend, subtrahend);

class BaseDispatcherTest : public ::testing::Test {
public:
    BaseDispatcherTest();
    wwa::json_rpc::dispatcher& dispatcher() noexcept { return this->m_dispatcher; }

private:
    wwa::json_rpc::dispatcher m_dispatcher;

    [[nodiscard]] int subtract(const subtract_params& params) const;
    [[nodiscard]] int subtract_p(int minuend, int subtrahend) const;
    void notification() const;
    [[nodiscard]] int no_params() const noexcept;
    [[nodiscard]] int sum(int a, int b, int c) const;
    [[nodiscard]] int sumv(const nlohmann::json& params) const;
    [[nodiscard]] nlohmann::json get_data() const;

    [[nodiscard]] static int s_sumv(const nlohmann::json& params);
    [[nodiscard]] static int s_subtract(const subtract_params& params);
    [[nodiscard]] static int s_subtract_p(int minuend, int subtrahend);
    static void s_notification();
};

#endif /* DF466239_E1AA_4975_87F2_5A43B9623F91 */
