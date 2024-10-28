#ifndef DE443A53_EEA9_4918_BCFB_AE76A19FB197
#define DE443A53_EEA9_4918_BCFB_AE76A19FB197

#include <functional>
#include <tuple>

namespace wwa::json_rpc::details {

template<typename T>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
    using return_type = R;
    using args_tuple  = std::tuple<Args...>;
};

template<typename R, typename... Args>
struct function_traits<R (*)(Args...) noexcept> : function_traits<R (*)(Args...)> {};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...)> {
    using return_type = R;
    using args_tuple  = std::tuple<Args...>;
};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R (C::*)(Args...)> {};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) volatile> : function_traits<R (C::*)(Args...)> {};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile> : function_traits<R (C::*)(Args...)> {};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) noexcept> : function_traits<R (C::*)(Args...)> {};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const noexcept> : function_traits<R (C::*)(Args...)> {};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) volatile noexcept> : function_traits<R (C::*)(Args...)> {};

template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile noexcept> : function_traits<R (C::*)(Args...)> {};

template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
    using return_type = R;
    using args_tuple  = std::tuple<Args...>;
};

template<typename T>
struct function_traits {
private:
    using call_type = function_traits<decltype(&T::operator())>;

public:
    using return_type = typename call_type::return_type;
    using args_tuple  = typename call_type::args_tuple;
};

}  // namespace wwa::json_rpc::details

#endif /* DE443A53_EEA9_4918_BCFB_AE76A19FB197 */
