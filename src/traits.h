#ifndef DE443A53_EEA9_4918_BCFB_AE76A19FB197
#define DE443A53_EEA9_4918_BCFB_AE76A19FB197

/**
 * @file traits.h
 * @brief Contains type traits for function handling in the JSON RPC library.
 * @internal
 */

#include <functional>
#include <tuple>

/**
 * @brief Contains the implementation details of the JSON RPC library.
 * @internal
 */
namespace wwa::json_rpc::details {

/**
 * @addtogroup function_traits Function Traits
 * @brief Type traits for extracting function return types and argument types.
 * @internal
 * @{
 */

/**
 * @brief Primary template for function traits.
 *
 * This template is specialized for various function types to extract the return type and argument types.
 *
 * @tparam T The function type.
 */
template<typename T>
struct function_traits;

/**
 * @brief Specialization for function pointers.
 *
 * @tparam R The return type of the function.
 * @tparam Args The argument types of the function.
 */
template<typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
    using return_type = R;                    ///< The return type of the function.
    using args_tuple  = std::tuple<Args...>;  ///< A tuple of the argument types.
};

/**
 * @brief Specialization for noexcept function pointers.
 *
 * @tparam R The return type of the function.
 * @tparam Args The argument types of the function.
 */
template<typename R, typename... Args>
struct function_traits<R (*)(Args...) noexcept> : function_traits<R (*)(Args...)> {};

/**
 * @brief Specialization for member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...)> {
    using return_type = R;                    ///< The return type of the member function.
    using args_tuple  = std::tuple<Args...>;  ///< A tuple of the argument types.
};

/**
 * @brief Specialization for const member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R (C::*)(Args...)> {};

/**
 * @brief Specialization for volatile member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) volatile> : function_traits<R (C::*)(Args...)> {};

/**
 * @brief Specialization for const volatile member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile> : function_traits<R (C::*)(Args...)> {};

/**
 * @brief Specialization for noexcept member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) noexcept> : function_traits<R (C::*)(Args...)> {};

/**
 * @brief Specialization for const noexcept member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const noexcept> : function_traits<R (C::*)(Args...)> {};

/**
 * @brief Specialization for volatile noexcept member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) volatile noexcept> : function_traits<R (C::*)(Args...)> {};

/**
 * @brief Specialization for const volatile noexcept member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile noexcept> : function_traits<R (C::*)(Args...)> {};

/**
 * @brief Specialization for `std::function`.
 *
 * @tparam R The return type of the function.
 * @tparam Args The argument types of the function.
 */
template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
    using return_type = R;                    ///< The return type of the function.
    using args_tuple  = std::tuple<Args...>;  ///< A tuple of the argument types.
};

/**
 * @brief Specialization for functors (objects with `operator()`).
 *
 * @tparam T The functor type.
 */
template<typename T>
struct function_traits {
private:
    using call_type = function_traits<decltype(&T::operator())>;  ///< Type traits for the call operator.

public:
    using return_type = typename call_type::return_type;  ///< The return type of the functor.
    using args_tuple  = typename call_type::args_tuple;   ///< A tuple of the argument types.
};
/** @} */

}  // namespace wwa::json_rpc::details

#endif /* DE443A53_EEA9_4918_BCFB_AE76A19FB197 */
