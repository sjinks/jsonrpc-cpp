#include "base.h"

#include <stdexcept>
#include <gtest/gtest.h>

BaseDispatcherTest::BaseDispatcherTest()
{
    this->m_dispatcher.add("subtract", &BaseDispatcherTest::subtract, this);
    this->m_dispatcher.add("subtract_p", &BaseDispatcherTest::subtract_p, this);
    this->m_dispatcher.add("notification", &BaseDispatcherTest::notification, this);

    this->m_dispatcher.add("s_subtract", &BaseDispatcherTest::s_subtract);
    this->m_dispatcher.add("s_subtract_p", &BaseDispatcherTest::s_subtract_p);
    this->m_dispatcher.add("s_notification", &BaseDispatcherTest::s_notification);

    this->m_dispatcher.add("no_params", &BaseDispatcherTest::no_params, this);

    this->m_dispatcher.add("sum", &BaseDispatcherTest::sum, this);
    this->m_dispatcher.add("get_data", &BaseDispatcherTest::get_data, this);
    this->m_dispatcher.add("notify_hello", [](int) { /* Do nothing */ });

    this->m_dispatcher.add("sumv", &BaseDispatcherTest::sumv, this);
    this->m_dispatcher.add("s_sumv", &BaseDispatcherTest::s_sumv);
    this->m_dispatcher.add("throwing", []() { throw std::invalid_argument("test"); });
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
int BaseDispatcherTest::subtract(const subtract_params& params) const
{
    return params.minuend - params.subtrahend;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
int BaseDispatcherTest::subtract_p(int minuend, int subtrahend) const
{
    return minuend - subtrahend;
}

void BaseDispatcherTest::notification() const
{
    /* Do nothing */
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
int BaseDispatcherTest::no_params() const noexcept
{
    return 24;  // NOLINT(readability-magic-numbers)
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
int BaseDispatcherTest::sum(int a, int b, int c) const
{
    return a + b + c;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
int BaseDispatcherTest::sumv(const nlohmann::json& params) const
{
    return BaseDispatcherTest::s_sumv(params);
}

int BaseDispatcherTest::s_sumv(const nlohmann::json& params)
{
    std::vector<int> v;
    params.get_to(v);
    return std::accumulate(v.begin(), v.end(), 0);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
nlohmann::json BaseDispatcherTest::get_data() const
{
    return nlohmann::json::array({"hello", 5});  // NOLINT(readability-magic-numbers)
}

int BaseDispatcherTest::s_subtract(const subtract_params& params)
{
    return params.minuend - params.subtrahend;
}

int BaseDispatcherTest::s_subtract_p(int minuend, int subtrahend)
{
    return minuend - subtrahend;
}

void BaseDispatcherTest::s_notification()
{
    /* Do nothing */
}
