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

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <nlohmann/json.hpp>

#include "exception.h"
#include "export.h"
#include "traits.h"

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
 * The dispatcher class also provides virtual methods that can be overridden in derived classes to customize
 * the behavior when a request is received, before a method handler is called, and after a method handler is called.
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
 * std::string request = R"({"jsonrpc": "2.0", "method": "subtract", "params": {"minuend": 42, "subtrahend": 23}, "id": 1})";
 * std::string response = d.parse_and_process_request(request);
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
private:
    friend class dispatcher_private;
    using handler_t = std::function<nlohmann::json(const nlohmann::json&)>;  ///< Method handler type used internally.

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
     * @tparam F Type of the handler function (@a f).
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
     * this->m_dispatcher.add("sum", [](const nlohmann::json& params) {
     *     std::vector<int> v;
     *     params.get_to(v);
     *     return std::accumulate(v.begin(), v.end(), 0);
     * });
     * ```
     *
     * @par Returning Values from a Handler:
     * @li The handler can return any value as long as it can be converted to a `nlohmann::json` value. If there is no default conversion available,
     * the handler can either return a `nlohmann::json` value directly,
     * or use [a custom `to_json()` function](https://github.com/nlohmann/json?tab=readme-ov-file#arbitrary-types-conversions).
     * @li If the handler function returns `void`, it will be automatically converted to `null` in the JSON response.
     *
     * @par Exception Handling:
     * @note If the hander function throws an exception (derived from `std::exception`), the exception will be caught, and the error will be returned in the JSON response:
     * @li json_rpc::exception will be converted to a JSON RPC error object using json_rpc::exception::to_json();
     * @li other exceptions derived from std::exception will be converted to a JSON RPC error object with code @a -32603 (exception::INTERNAL_ERROR)
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
        using traits             = details::function_traits<std::decay_t<F>>;
        using ArgsTuple          = typename traits::args_tuple;
        constexpr auto args_size = std::tuple_size<ArgsTuple>();

        auto&& closure =
            this->create_closure<C, F, ArgsTuple>(instance, std::forward<F>(f), std::make_index_sequence<args_size>{});
        this->add_internal_method(method, std::forward<decltype(closure)>(closure));
    }

    /**
    * @brief Parses and processes a JSON RPC request.
    *
    * @param request The JSON RPC request as a string.
    * @return The response serialized into a JSON string.
    * @retval "" If the request is a [Notification](https://www.jsonrpc.org/specification#notification), the method returns an empty string.
    *
    * @details This method performs the following steps:
    * @li Parses the JSON RPC request.
    * @li Passes the parsed JSON to the process_request() method.
    * @li If the request is invalid, returns an appropriate error response.
    *
    * If the request cannot be parsed, the method returns a JSON RPC error response with code @a -32700 (exception::PARSE_ERROR).
    *
    * Exceptions derived from `std::exception` thrown by the handler function are caught and returned as JSON RPC error responses.
    * @li `json_rpc::exception` will be converted to a JSON RPC error object using `json_rpc::exception::to_json()`.
    * @li Other exceptions derived from `std::exception` will be converted to a JSON RPC error object with code `-32603` (exception::INTERNAL_ERROR) and the exception message as the error message.
    */
    std::string parse_and_process_request(const std::string& request);

    /**
     * @brief Processes a JSON RPC request.
     *
     * @param request The JSON RPC request as a `nlohmann::json` object.
     * @return The response serialized into a JSON string.
     * @retval "" If the request is a [Notification](https://www.jsonrpc.org/specification#notification), the method returns an empty string.
     *
     * @details This method performs the following steps:
     * @li Parses the JSON RPC request.
     * @li Passes the parsed JSON to the appropriate handler function.
     * @li If the request is invalid, returns an appropriate error response.
     *
     * If the request cannot be parsed, the method returns a JSON RPC error response with code `-32700` (exception::PARSE_ERROR).
     *
     * Exceptions derived from `std::exception` thrown by the handler function are caught and returned as JSON RPC error responses:
     * @li `json_rpc::exception` will be converted to a JSON RPC error object using `json_rpc::exception::to_json()`.
     * @li Other exceptions derived from `std::exception` will be converted to a JSON RPC error object with code `-32603` (exception::INTERNAL_ERROR) and the exception message as the error message.
     */
    std::string process_request(const nlohmann::json& request);

    /**
     * @brief Invoked when a request is received.
     *
     * @details This method does nothing by default. It is intended to be overridden in a derived class.
     * For example, it can be used to log requests or increment a counter.
     *
     * @note In the case of a valid [batch request](https://www.jsonrpc.org/specification#batch),
     * this method is invoked for every request in the batch but **not** for the batch itself.
     * However, if the batch request is invalid (e.g., is empty), this method is invoked once with an empty method name.
     */
    virtual void on_request();

    /**
     * @brief Invoked right before the method handler is called.
     *
     * @details This method does nothing by default. It is intended to be overridden in a derived class. For example, it can start a timer to measure the method execution time.
     *
     * @param method The name of the method to be called.
     */
    virtual void on_method(const std::string& method);

    /**
     * @brief Invoked after the method handler is called.
     *
     * @details This method does nothing by default. It is intended to be overridden in a derived class.
     * For example, it can be used to stop the timer started in on_method().
     *
     * @param method The name of the called method. It can be empty for an invalid batch request.
     * @param code The result code: 0 if the method was processed successfully, or an error code
     * if an exception was thrown (e.g., exception::INTERNAL_ERROR)
     * or the request could not be processed (e.g., exception::INVALID_PARAMS).
     */
    virtual void on_request_processed(const std::string& method, int code);

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
     * @brief Invokes a handler function with the provided arguments.
     *
     * @tparam F The type of the handler function.
     * @tparam Tuple The type of the arguments tuple.
     * @param f The handler function.
     * @param tuple The arguments as a tuple.
     * @return The result of the handler function converted to a JSON value.
     * @retval nlohmann::json::null_t If the handler function returns void.
     *
     * @details This method invokes the handler function with the arguments passed as a tuple.
     * It uses the `if constexpr` construct to handle the case when the handler function returns void.
     *
     * The `if constexpr` construct allows the method to determine at compile time whether
     * the handler function returns void. If the return type is void,
     * the method calls the handler function and returns a JSON null value.
     * If the return type is not void, the method calls the handler function
     * and returns the result converted to a JSON value.
     *
     * The `std::apply` function is used to unpack the tuple and pass the arguments to the handler function.
     */
    template<typename F, typename Tuple>
    nlohmann::json invoke_function(F&& f, Tuple&& tuple) const
    {
        using ReturnType = typename details::function_traits<std::decay_t<F>>::return_type;

        if constexpr (std::is_void_v<ReturnType>) {
            std::apply(std::forward<F>(f), std::forward<Tuple>(tuple));
            // NOLINTNEXTLINE(modernize-return-braced-init-list) -- braced init will create a JSON array
            return nlohmann::json(nullptr);
        }
        else {
            return std::apply(std::forward<F>(f), std::forward<Tuple>(tuple));
        }
    }

    /**
     * @brief Creates a closure for invoking a member function with JSON parameters.
     *
     * @tparam C The type of the class instance (can be a pointer or null pointer).
     * @tparam F The type of the member function (if C is not `std::nullptr_t`) or the function.
     * @tparam Args The type of the arguments tuple.
     * @tparam Indices The indices of the arguments in the tuple.
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
    template<typename C, typename F, typename Args, std::size_t... Indices>
    constexpr auto
    create_closure(C inst, F&& f, std::index_sequence<Indices...>)  // NOLINT(readability-function-cognitive-complexity)
    {
        static_assert(std::is_pointer_v<C> || std::is_null_pointer_v<C>);
        return [func = std::forward<F>(f), inst, this](const nlohmann::json& params) {
            constexpr auto args_size = std::tuple_size<Args>();

            if (params.is_array()) {
                if constexpr (args_size == 1) {
                    if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<0, Args>>, nlohmann::json>) {
                        auto&& tuple_args = [inst, &params]() constexpr {
                            (void)inst;
                            if constexpr (std::is_null_pointer_v<C>) {
                                return std::make_tuple(params);
                            }
                            else {
                                return std::make_tuple(inst, params);
                            }
                        }();

                        return this->invoke_function(func, std::forward<decltype(tuple_args)>(tuple_args));
                    }
                }

                if (params.size() == std::tuple_size<Args>()) {
                    auto&& tuple_args = [inst, &params]() constexpr {
                        (void)inst;
                        (void)params;
                        try {
                            if constexpr (std::is_null_pointer_v<C>) {
                                return std::make_tuple(
                                    params[Indices].get<std::decay_t<std::tuple_element_t<Indices, Args>>>()...
                                );
                            }
                            else {
                                return std::make_tuple(
                                    inst, params[Indices].get<std::decay_t<std::tuple_element_t<Indices, Args>>>()...
                                );
                            }
                        }
                        catch (const nlohmann::json::exception& e) {
                            throw exception(exception::INVALID_PARAMS, e.what());
                        }
                    }();

                    return this->invoke_function(func, std::forward<decltype(tuple_args)>(tuple_args));
                }
            }

            throw exception(exception::INVALID_PARAMS, err_invalid_params_passed_to_method);
        };
    }
};

}  // namespace wwa::json_rpc

#endif /* FAB131EA_3F90_43B6_833D_EB89DA373735 */
