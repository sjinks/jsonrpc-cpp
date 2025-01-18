#ifndef FAB131EA_3F90_43B6_833D_EB89DA373735
#define FAB131EA_3F90_43B6_833D_EB89DA373735

/**
 * @file dispatcher.h
 * @brief Defines the JSON RPC dispatcher class.
 *
 * This file contains the definition of the `dispatcher` class, which is responsible for managing JSON RPC method handlers
 * and processing JSON RPC requests. The dispatcher supports adding various types of handlers, including plain functions,
 * static class methods, lambda functions, and member functions. These handlers can accept and return values that are
 * convertible to and from `nlohmann::json` values.
 */

#include <any>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <nlohmann/json.hpp>

#include "details.h"
#include "exception.h"
#include "export.h"

/**
 * @brief Library namespace.
 */
namespace wwa::json_rpc {

class dispatcher_private;

/**
 * @brief A class that manages JSON RPC method handlers and processes JSON RPC requests.
 *
 * The dispatcher class allows adding method handlers for JSON RPC methods and processes JSON RPC requests.
 * It supports adding plain functions, static class methods, lambda functions, and member functions as handlers.
 * The handlers can accept and return values that can be converted to and from `nlohmann::json` values.
 *
 * @note The dispatcher class is non-copyable but movable.
 *
 * @details
 * The dispatcher class provides the following functionalities:
 * - Adding method handlers for JSON RPC methods.
 * - Parsing and processing JSON RPC requests.
 * - Invoking method handlers with the appropriate arguments.
 * - Handling exceptions thrown by method handlers and returning appropriate JSON RPC error responses.
 *
 * @par Example Usage:
 * ```cpp
 * dispatcher d;
 * d.add("subtract", [](const nlohmann::json& params) {
 *     int minuend    = params["minuend"];
 *     int subtrahend = params["subtrahend"];
 *     return minuend - subtrahend;
 * });
 *
 * const auto request  = R"({"jsonrpc": "2.0", "method": "subtract", "params": {"minuend": 42, "subtrahend": 23}, "id": 1})";
 * const auto response = d.process_request(nlohmann::json::parse(request)).dump();
 * ```
 *
 * @par Adding Method Handlers:
 * Method handlers can be added using the `add` method. The handler function can accept any number of arguments
 * as long as they can be converted from a `nlohmann::json` value. The handler function can also return any type
 * that can be converted to a `nlohmann::json` value.
 *
 * @par Handling Exceptions:
 * If a handler function throws an exception derived from `std::exception`, the exception will be caught and
 * an appropriate JSON RPC error response will be returned.
 */
class WWA_JSONRPC_EXPORT dispatcher {
public:
    /**
     * @brief Optional context data for method handlers.
     *
     * @details This type alias defines a context data type that can be passed to method handlers.
     * The context data is a pair of two values:
     *   - The first value is an `std::any` object that is passed to the `process_request()` method;
     *   - The second value is a `nlohmann::json` object that contains additional fields extracted from the JSON RPC request.
     */
    using context_t = std::pair<std::any, nlohmann::json>;

private:
    friend class dispatcher_private;

    /**
     * @brief Method handler type.
     *
     * @details This type alias defines a method handler function that takes two parameters:
     * - `ctx`: An additional parameter that can be used to pass additional information to the handler.
     * - `params`: A JSON object containing the parameters for the method.
     *
     * The handler function returns a JSON object as a result.
     *
     * This type alias is used to define the signature of functions that handle method calls in the dispatcher.
     */
    using handler_t = std::function<nlohmann::json(const context_t& ctx, const nlohmann::json& params)>;

public:
    /** @brief Class constructor. */
    dispatcher();

    /** @brief Class destructor. */
    virtual ~dispatcher();

    dispatcher(const dispatcher&)            = delete;
    dispatcher& operator=(const dispatcher&) = delete;

    /**
     * @brief Move constructor.
     * @param rhs Right-hand side object.
     */
    dispatcher(dispatcher&& rhs) = default;

    /**
     * @brief Move assignment operator.
     * @param rhs Right-hand side object.
     * @return Reference to this object.
     */
    dispatcher& operator=(dispatcher&& rhs) = default;

    /**
     * @brief Adds a method handler @a f for the method @a method.
     * @tparam F Type of the handler function @a f.
     * @param method The name of the method to add the handler for.
     * @param f The handler function.
     * @details This overload is used to add a plain function, a static class method, or a lambda function as a handler.
     *
     * Internally, the handler is a function that accepts a `nlohmann::json` as its argument and returns a `nlohmann::json` value.
     * However, the handler function can accept any number of arguments, as long as they
     * [can be converted](https://github.com/nlohmann/json?tab=readme-ov-file#arbitrary-types-conversions) from a `nlohmann::json` value.
     * The same is true for the return value: it can be any type that can be converted to a `nlohmann::json` value (or `void`).
     * Of course, the handler can accept and/or return `nlohmann::json` values directly.
     *
     * @par Accepting Arguments in a Handler:
     * The Specification [defines](https://www.jsonrpc.org/specification#parameter_structures) two types of parameters: *named*  and *positional*.
     * Since there is no easy way to match named parameters to the handler function arguments, the named parameters are treated as a single structured value.
     * @par
     * For example, if the parameters are passed like this: `"params": {"subtrahend": 23, "minuend": 42}`, the handler function must accept a single argument of a struct type:
     * ```cpp
     * struct subtract_params {
     *     int minuend;
     *     int subtrahend;
     * };
     *
     * int subtract(const subtract_params& params)
     * {
     *     return params.minuend - params.subtrahend;
     * }
     * ```
     * @par
     * Because there is no automatic conversion from a JSON object to `subtract_params`, you must define the conversion function yourself.
     * The easiest way is to [use a macro](https://github.com/nlohmann/json?tab=readme-ov-file#simplify-your-life-with-macros):
     * ```cpp
     * NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(subtract_params, minuend, subtrahend);
     * ```
     * @par
     * In the case of positional parameters, the handler function must accept the same number of arguments as the number of parameters passed in the request.
     * @par
     * If the handler needs to accept a variable number of arguments, it must accept a single `nlohmann::json` argument and parse it as needed, like this:
     * ```cpp
     * dispatcher.add("sum", [](const nlohmann::json& params) {
     *     std::vector<int> v;
     *     params.get_to(v);
     *     return std::accumulate(v.begin(), v.end(), 0);
     * });
     * ```
     * @note If the handler accepts a single `nlohmann::json` argument, it will accept *any* parameters. For example:
     * ```cpp
     * void handler(const nlohmann::json& params)
     * {
     *     if (params.is_null()) {
     *         // No parameters
     *     }
     *     else if (params.is_array()) {
     *         // Array of positional parameters
     *     }
     *     else if (params.is_object()) {
     *         // Named parameters
     *     }
     * }
     * ```
     *
     * @par Returning Values from a Handler:
     * 1. The handler can return any value as long as it can be converted to a `nlohmann::json` value. If there is no default conversion available,
     * the handler can either return a `nlohmann::json` value directly,
     * or use [a custom `to_json()` function](https://github.com/nlohmann/json?tab=readme-ov-file#arbitrary-types-conversions).
     * 2. If the handler function returns `void`, it will be automatically converted to `null` in the JSON response.
     *
     * @par Exception Handling:
     * If the hander function throws an exception (derived from `std::exception`), the exception will be caught, and the error will be returned in the JSON response:
     * 1. `json_rpc::exception` will be converted to a JSON RPC error object using json_rpc::exception::to_json();
     * 2. other exceptions derived from `std::exception` will be converted to a JSON RPC error object with code @a -32603 (`exception::INTERNAL_ERROR`)
     * and the exception message ([what()](https://en.cppreference.com/w/cpp/error/exception/what)) as the error message.
     */
    template<typename F>
    void add(std::string_view method, F&& f)
    {
        this->add(method, std::forward<F>(f), nullptr);
    }

    /**
     * @brief Adds a method to the dispatcher with the specified instance and function.
     * @tparam C The type of the class instance.
     * @tparam F The type of the function to be added.
     * @param method The name of the method to be added.
     * @param f The function to be added, which will be bound to the instance.
     * @param instance The instance of the class to which the function belongs.
     * @overload
     * @details This template method allows adding a method to the dispatcher by binding a member function
     * of a class instance. It uses function traits to deduce the argument types and creates a closure
     * that is then added to the internal method map.
     */
    template<typename C, typename F>
    void add(std::string_view method, F&& f, C instance)
    {
        using traits    = details::function_traits<std::decay_t<F>>;
        using ArgsTuple = typename traits::args_tuple;

        const auto&& closure = this->create_closure<C, F, void, ArgsTuple>(instance, std::forward<F>(f));
        this->add_internal_method(method, std::forward<decltype(closure)>(closure));
    }

    /**
     * @brief Adds a method handler with a context parameter.
     *
     * @tparam F The type of the handler function.
     * @param method The name of the method to add the handler for.
     * @param f The handler function.
     *
     * @details This method allows adding a handler function with an additional context parameter.
     * The context parameter can be used to pass additional information to the handler function.
     * The handler function can accept any number of arguments as long as they can be converted from a `nlohmann::json` value.
     * The same is true for the return value: it can be any type that can be converted to a `nlohmann::json` value (or `void`).
     *
     * This overload is used to add a plain function, a static class method, or a lambda function as a handler.
     *
     * @par Sample Usage:
     * ```cpp
     * struct extra_params {
     *     std::string ip;
     * };
     *
     * dispatcher.add_ex("sum", [](const std::any& extra, const nlohmann::json& params) {
     *     std::cout << "Invoking sum() method for " << std::any_cast<extra_params>(extra).ip << "\n";
     *     std::vector<int> v;
     *     params.get_to(v);
     *     return std::accumulate(v.begin(), v.end(), 0);
     * });
     * ```
     *
     * See `process_request()` for more details on the extra parameter.
     *
     * @see add()
     */
    template<typename F>
    void add_ex(std::string_view method, F&& f)
    {
        this->add_ex(method, std::forward<F>(f), nullptr);
    }

    /**
     * @brief Adds a method handler with a context parameter and a class instance.
     *
     * @tparam C The type of the class instance.
     * @tparam F The type of the handler function.
     * @param method The name of the method to add the handler for.
     * @param f The handler function.
     * @param instance The instance of the class to which the function belongs.
     *
     * @overload
     * @details This method allows adding a class method handler with an additional context parameter.
     * The context parameter can be used to pass additional information to the handler function.
     * The handler function can accept any number of arguments as long as they can be converted from a `nlohmann::json` value.
     * The same is true for the return value: it can be any type that can be converted to a `nlohmann::json` value (or `void`).
     *
     * @see add()
     */
    template<typename C, typename F>
    void add_ex(std::string_view method, F&& f, C instance)
    {
        using traits    = details::function_traits<std::decay_t<F>>;
        using ArgsTuple = typename traits::args_tuple;

        static_assert(
            std::tuple_size<std::decay_t<ArgsTuple>>::value > 0,
            "Handler function must accept the `context` argument. Use `add()` for handlers without the `context` "
            "argument."
        );

        const auto&& closure = this->create_closure<C, F, context_t, ArgsTuple>(instance, std::forward<F>(f));
        this->add_internal_method(method, std::forward<decltype(closure)>(closure));
    }

    /**
     * @brief Processes a JSON RPC request.
     *
     * @param request The JSON RPC request as a `nlohmann::json` object.
     * @param data Optional data that can be passed to the handler function (only for handlers added with @a add_ex()).
     * @return The response as a `nlohmann::json` object. If the request is a [Notification](https://www.jsonrpc.org/specification#notification),
     * it will be of the [discarded_t](https://json.nlohmann.me/api/basic_json/is_discarded/) type.
     *
     * @details This method processes a JSON RPC request.
     *
     * Exceptions derived from `std::exception` thrown by the handler function are caught and returned as JSON RPC error responses:
     * @li `json_rpc::exception` will be converted to a JSON RPC error object using `json_rpc::exception::to_json()`.
     * @li Other exceptions derived from `std::exception` will be converted to a JSON RPC error object with code `-32603` (exception::INTERNAL_ERROR)
     * and the exception message as the error message.
     *
     * For the handlers that accept the `context` parameter, this method will construct the context as follows:
     * 1. The `data` parameter will be passed as the first element of the `context` tuple.
     * 2. The extra fields from the JSON RPC request will be passed as the second element of the `context` tuple as a JSON object.
     *
     * For example, given this request:
     * ```json
     * {
     *     "jsonrpc": "2.0",
     *     "method": "subtract",
     *     "params": {"minuend": 42, "subtrahend": 23},
     *     "id": 1,
     *     "auth": "secret",
     *     "user": "admin"
     * }
     * ```
     * and `data` set to `std::string("some_data")`, the `context` parameter passed to the handler will be a pair of values:
     *   - `std::string("some_data")` as `std::any`;
     *   - `nlohmann::json` representing the object `{ "auth": "secret", "user": "admin" }`.
     */
    nlohmann::json process_request(const nlohmann::json& request, const std::any& data = {});

protected:
    virtual nlohmann::json do_process_request(const nlohmann::json& request, const std::any& data, bool is_batch);

    /**
     * @brief Processes a batch request.
     *
     * @param request The batch request as a `nlohmann::json` array.
     * @param data Additional information to pass to the method handlers as a part of the context.
     * @return The response as a JSON array.
     *
     * @details This method processes a batch request by invoking the method handlers for each request in the batch.
     */
    virtual nlohmann::json process_batch_request(const nlohmann::json& request, const std::any& data);

    /**
     * @brief Invokes a method handler.
     *
     * @param method The name of the method to invoke.
     * @param params The parameters for the method.
     * @param ctx The context to pass to the method handlers.
     * @return The result of the method invocation as a JSON object.
     *
     * @details This method finds the handler for the specified method and invokes it with the provided parameters.
     * @throws exception If the method is not found or the invocation fails.
     * @see exception::METHOD_NOT_FOUND
     */
    virtual nlohmann::json
    invoke(const std::string& method, const nlohmann::json& params, const dispatcher::context_t& ctx);

    virtual void request_failed(const nlohmann::json& request_id, const std::exception* e, bool is_batch);

private:
    /**
     * @brief Pointer to the implementation (Pimpl idiom).
     *
     * @details This unique pointer holds the private implementation details of the dispatcher class.
     * It is used to hide the implementation details and reduce compilation dependencies.
     */
    std::unique_ptr<dispatcher_private> d_ptr;

    /**
     * @brief Adds a method handler for the specified method.
     *
     * @param method The name of the method.
     * @param handler The handler function.
     *
     * @details This method registers a handler function for a given method name.
     * The handler function will be invoked when a request for the specified method is received.
     */
    void add_internal_method(std::string_view method, handler_t&& handler);

    /**
     * @brief Creates a closure for invoking a member function with JSON parameters.
     *
     * @tparam C The type of the class instance (can be a pointer or null pointer).
     * @tparam F The type of the member function (if C is not `std::nullptr_t`) or the function.
     * @tparam Context The type of the context parameter that can be passed to the member function (can be `void` or `context_t`).
     * @tparam Args The type of the arguments tuple.
     *
     * @param inst The instance of the class (can be a pointer or null pointer).
     * @param f The member function to be invoked.
     *
     * @return A lambda function that takes a JSON object as a parameter and invokes the member function with the appropriate arguments.
     *
     * @details This method creates a closure (lambda function) that can be used to invoke a member function with arguments extracted from a JSON object.
     *
     * The closure performs the following steps:
     * 1. Checks if the JSON object is an array.
     * 2. If the JSON object is an array and the member function takes a single argument of type `nlohmann::json`, it directly passes the JSON object to the member function.
     * 3. If the JSON object is an array and the number of elements matches the number of arguments expected by the member function, it extracts the arguments from the JSON array and invokes the member function.
     * 4. If the JSON object is not an array or the number of elements does not match the number of arguments, it throws a json_rpc::exception with the  `exception::INVALID_PARAMS` code.
     *
     * The `invoke_function` method is used to invoke the member function with the extracted arguments.
     *
     * The `std::apply` function is used to unpack the tuple and pass the arguments to the member function.
     *
     * Compile-time checks ensure that the code is type-safe and that certain conditions are met before the code is compiled.
     * This helps catch potential errors early in the development process and improves the overall robustness of the code.
     */
    template<typename C, typename F, typename Context, typename Args>
    constexpr auto create_closure(C inst, F&& f) const
    {
        static_assert((std::is_pointer_v<C> && std::is_class_v<std::remove_pointer_t<C>>) || std::is_null_pointer_v<C>);
        return [func = std::forward<F>(f), inst](const context_t& ctx, const nlohmann::json& params) {
            assert(params.is_array());
            constexpr auto args_size = std::tuple_size<std::decay_t<Args>>::value;
            constexpr auto arg_pos   = std::is_void_v<Context> ? 0 : 1;

            if constexpr (args_size == arg_pos + 1) {
                if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<arg_pos, Args>>, nlohmann::json>) {
                    auto&& tuple_args = std::tuple_cat(
                        details::make_inst_tuple(inst), details::make_context_tuple<Context>(ctx),
                        std::make_tuple(params)
                    );

                    return details::invoke_function(func, std::forward<decltype(tuple_args)>(tuple_args));
                }
            }

            if (params.size() + arg_pos == args_size) {
                constexpr auto offset = std::is_void_v<Context> ? 0U : 1U;
                auto&& tuple_args     = std::tuple_cat(
                    details::make_inst_tuple(inst), details::make_context_tuple<Context>(ctx),
                    details::convert_args<Context, Args>(
                        params, details::offset_sequence_t<offset, std::make_index_sequence<args_size - offset>>{}
                    )
                );

                return details::invoke_function(func, std::forward<decltype(tuple_args)>(tuple_args));
            }

            throw exception(exception::INVALID_PARAMS, err_invalid_params_passed_to_method);
        };
    }
};

}  // namespace wwa::json_rpc

#endif /* FAB131EA_3F90_43B6_833D_EB89DA373735 */
