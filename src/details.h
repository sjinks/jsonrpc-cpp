#ifndef DE443A53_EEA9_4918_BCFB_AE76A19FB197
#define DE443A53_EEA9_4918_BCFB_AE76A19FB197

/**
 * @file
 * @brief Contains implementation details for the JSON RPC library.
 *
 * This file includes type traits for extracting function return types and argument types,
 * as well as utilities for creating offset sequences and handling handler parameters.
 * These details are used internally by the JSON RPC library to facilitate various operations.
 *
 * It includes:
 * - Type traits for function return types and argument types.
 * - Utilities for creating offset sequences.
 * - Helper functions for handling handler parameters.
 *
 * @internal
 */

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <nlohmann/json.hpp>
#include "exception.h"

/**
 * @brief Contains the implementation details of the JSON RPC library.
 * @internal
 */
namespace wwa::json_rpc::details {

/**
 * @defgroup function_traits Function Traits
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
 * @tparam NX Whether the function is noexcept.
 * @tparam Args The argument types of the function.
 */
template<typename R, bool NX, typename... Args>
struct function_traits<R (*)(Args...) noexcept(NX)> {
    using return_type = R;                    ///< The return type of the function.
    using args_tuple  = std::tuple<Args...>;  ///< A tuple of the argument types.
};

/**
 * @brief Specialization for member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) noexcept(NX)> : function_traits<R (*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for const member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) const noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for volatile member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) volatile noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for const volatile member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile noexcept(NX)>
    : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for lvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) & noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for const lvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) const & noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for volatile lvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) volatile & noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for const volatile lvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile & noexcept(NX)>
    : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for rvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) && noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for const rvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) const && noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for volatile rvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) volatile && noexcept(NX)> : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for const volatile rvalue reference member function pointers.
 *
 * @tparam R The return type of the member function.
 * @tparam C The class type.
 * @tparam NX Whether the member function is noexcept.
 * @tparam Args The argument types of the member function.
 */
template<typename R, typename C, bool NX, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile && noexcept(NX)>
    : function_traits<R (C::*)(Args...) noexcept(NX)> {};

/**
 * @brief Specialization for `std::function`.
 *
 * @tparam S The type of the function, which includes the return type and argument types.
 */
template<typename S>
struct function_traits<std::function<S>> : function_traits<std::decay_t<S>> {};

/**
 * @brief Specialization for functors (objects with `operator()`).
 *
 * @tparam T The functor type.
 */
template<typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};
/** @} */

/**
 * @defgroup offset_sequence Offset Sequence
 * @brief Utilities for creating offset sequences.
 * @see https://devblogs.microsoft.com/oldnewthing/20200625-00/?p=103903
 * @internal
 * @{
 */

/**
 * @brief Template for creating an offset sequence.
 *
 * @tparam N The offset value.
 * @tparam Seq The sequence type.
 */
template<std::size_t N, typename Seq>
struct offset_sequence;

/**
 * @brief Specialization for creating an offset sequence.
 *
 * @tparam N The offset value.
 * @tparam Ints The integer sequence.
 */
template<std::size_t N, std::size_t... Ints>
struct offset_sequence<N, std::index_sequence<Ints...>> {
    using type = std::index_sequence<Ints + N...>;  ///< The resulting offset sequence.
};

/**
 * @brief Alias template for creating an offset sequence.
 *
 * @tparam N The offset value.
 * @tparam Seq The sequence type.
 */
template<std::size_t N, typename Seq>
using offset_sequence_t = typename offset_sequence<N, Seq>::type;

/** @} */

/**
 * @defgroup param_helpers Parameter Helpers
 * @brief Utilities for handling handler parameters.
 * @internal
 * @{
 */

/**
 * @brief Creates a tuple containing the instance if it is a class pointer, or an empty tuple if it is a null pointer.
 *
 * @tparam C The type of the instance, which should be a class pointer or a null pointer.
 * @param inst The instance to be included in the tuple.
 * @return A tuple containing the instance if it is a class pointer, or an empty tuple if it is a null pointer.
 *
 * @note This function uses `constexpr` and `if constexpr` to ensure that the tuple is created at compile time if possible.
 * @note The function requires that the type C is either a class pointer or a null pointer.
 */
template<typename C>
constexpr auto make_inst_tuple(C inst)
requires(std::is_class_v<std::remove_pointer_t<C>> || std::is_null_pointer_v<C>)
{
    if constexpr (std::is_null_pointer_v<C>) {
        return std::make_tuple();
    }
    else {
        return std::make_tuple(inst);
    }
}

/**
 * @brief Creates a tuple from the provided context object based on the type of @a Context.
 *
 * @tparam Context The type of the context.
 * @param ctx The context object from which the tuple is created.
 * @return A tuple containing the context object.
 *
 * @throws wwa::json_rpc::exception If the extraction of @a Extra from the JSON object @a extra fails.
 */
template<typename Context>
constexpr auto make_context_tuple([[maybe_unused]] const std::pair<std::any, nlohmann::json>& ctx)
{
    if constexpr (std::is_void_v<Context>) {
        return std::make_tuple();
    }
    else {
        return std::make_tuple(ctx);
    }
}

/**
 * @brief Invokes a function with the provided arguments handling `void` return type.
 *
 * @tparam F The type of the function.
 * @tparam Tuple The type of the arguments tuple.
 * @param f The function.
 * @param tuple The arguments as a tuple.
 * @return The result of the function converted to a JSON value.
 * @retval nlohmann::json::null_t If the function returns void.
 *
 * @details This helper invokes the function with the arguments passed as a tuple.
 * It uses the `if constexpr` construct to handle the case when the handler function returns void.
 *
 * The `if constexpr` construct allows for determinining at compile time whether @a f returns `void`.
 * If the return type is `void`, `invoke_function` calls @a f and returns a JSON `null` value.
 * If the return type is not `void`, `invoke_function` calls @a f and returns the result converted to a JSON value.
 */
template<typename F, typename Tuple>
nlohmann::json invoke_function(F&& f, Tuple&& tuple)
{
    using ReturnType = typename details::function_traits<std::decay_t<F>>::return_type;

    if constexpr (std::is_void_v<ReturnType>) {
        std::apply(std::forward<F>(f), std::forward<Tuple>(tuple));
        return nullptr;
    }
    else {
        return std::apply(std::forward<F>(f), std::forward<Tuple>(tuple));
    }
}

/**
 * @brief Type alias for a tuple element with decay applied.
 *
 * This template alias retrieves the type of the @a I-th element in the tuple type @a A,
 * and then applies `std::decay_t` to remove any references and cv-qualifiers.
 *
 * @tparam I The index of the element in the tuple.
 * @tparam A The tuple type.
 */
template<std::size_t I, typename A>
using tuple_element = std::decay_t<std::tuple_element_t<I, A>>;

/**
 * @brief Converts JSON parameters to a tuple of arguments based on the specified types.
 *
 * @tparam Extra An additional type that may be included in the conversion. If Extra is void, it is ignored.
 * @tparam Args A tuple of argument types to which the JSON parameters will be converted.
 * @tparam Indices A parameter pack representing the indices of the arguments.
 * @param params The JSON object containing the parameters to be converted.
 * @return A tuple containing the converted arguments.
 *
 * @throws wwa::json_rpc::exception If the conversion of any parameter from the JSON object fails.
 *
 * @details The function attempts to convert each parameter in the JSON object @a params to the corresponding type in @a Args and returns them as a tuple.
 * It uses `std::index_sequence` to unpack the indices and access the corresponding parameters in @a params.
 *
 * We support two types of handler functions:
 * 1. `ReturnType handler(const Extra& extra, Arguments... args);` (@a Extra is not `void`, @a Indices are [1..`sizeof...(Args)-1`])
 * 2. `ReturnType handler(Arguments... args);` (@a Extra is `void`, @a Indices are [0..`sizeof...(Args)-1`])
 *
 * Because @a Args correspond to the function arguments, @a Args contains both @a Extra and @a Arguments for the first type of handler; therefore, `Args[i]` will be the type of the `params[i-1]`.
 * For the second type of handler, @a Args contains only @a Arguments, and `Args[i]` will be the type of the `params[i]`.
 */
template<typename Extra, typename Args, std::size_t... Indices>
constexpr auto convert_args(const nlohmann::json& params, std::index_sequence<Indices...>)
{
    constexpr std::size_t offset = std::is_void_v<Extra> ? 0 : 1;
    try {
        return std::make_tuple(params[Indices - offset].template get<tuple_element<Indices, Args>>()...);
    }
    catch (const nlohmann::json::exception& e) {
        throw wwa::json_rpc::exception(wwa::json_rpc::exception::INVALID_PARAMS, e.what());
    }
}

/** @} */

}  // namespace wwa::json_rpc::details

#endif /* DE443A53_EEA9_4918_BCFB_AE76A19FB197 */
