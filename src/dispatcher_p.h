#ifndef FB656817_7041_48D5_80B2_347168163158
#define FB656817_7041_48D5_80B2_347168163158

/**
 * @file
 * @brief Contains the private implementation details of the JSON RPC dispatcher class.
 * @internal
 */

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include "dispatcher.h"

namespace wwa::json_rpc {

class exception;

/**
 * @brief Custom hasher for `std::string_view`.
 * @internal
 *
 * This struct provides a custom hash function for `std::string_view` to be used in unordered containers.
 */
struct hasher {
    using is_transparent = void;  ///< Indicates that the hasher supports transparent key lookup.

    /**
     * @brief Computes the hash value for a given `std::string_view`.
     *
     * @param s The `std::string_view` to hash.
     * @return The hash value.
     */
    std::size_t operator()(std::string_view s) const noexcept
    {
        const std::hash<std::string_view> h;
        return h(s);
    }
};

/**
 * @brief Represents a JSON RPC request.
 * @internal
 *
 * This struct holds the components of a JSON RPC request, including the JSON RPC version, method name, parameters, and ID.
 *
 * @see https://www.jsonrpc.org/specification#request_object
 */
struct jsonrpc_request {
    std::string jsonrpc;    ///< The JSON RPC version.
    std::string method;     ///< The name of the method to be invoked.
    nlohmann::json params;  ///< The parameters for the method.
    nlohmann::json id;      ///< The ID of the request.
};

/**
 * @brief Private implementation of the JSON RPC dispatcher class.
 * @internal
 *
 * This class contains the private members and methods used by the `dispatcher` class to manage method handlers and process requests.
 */
class dispatcher_private {
public:
    /**
     * @brief Constructs a new `dispatcher_private` object.
     *
     * @param q Pointer to the public `dispatcher` object.
     */
    explicit dispatcher_private(dispatcher* q) : q_ptr(q) {}

    /**
     * @brief Adds a method handler.
     *
     * @param method The name of the method.
     * @param handler The handler function.
     *
     * @details This method registers a handler function for a given method name.
     * The handler function will be invoked when a request for the specified method is received.
     */
    void add_handler(std::string&& method, dispatcher::handler_t&& handler);

    /**
     * @brief Processes a JSON RPC request.
     *
     * @param request The JSON RPC request as a `nlohmann::json` object.
     * @param extra The extra parameter to pass to the method handlers.
     * @return The response serialized into a JSON object.
     *
     * @details This method parses the JSON RPC request, validates it, and invokes the appropriate method handler.
     *
     * @see dispatcher::process_request()
     */
    nlohmann::json process_request(const nlohmann::json& request, const nlohmann::json& extra);

    /**
     * @brief Generates an error response.
     *
     * @param e The exception containing the error details.
     * @param id The ID of the request.
     * @return The error response serialized into a JSON object.
     *
     * @details This method creates a JSON RPC error response based on the provided exception and request ID.
     * @see exception::to_json()
     */
    static nlohmann::json generate_error_response(const exception& e, const nlohmann::json& id);

private:
    dispatcher* q_ptr;  ///< Pointer to the public `dispatcher` object.
    /** @brief Map of method names to handler functions. */
    std::unordered_map<std::string, dispatcher::handler_t, hasher, std::equal_to<>> m_methods;

    /**
     * @brief Parses a JSON RPC request.
     *
     * @param request The JSON RPC request as a `nlohmann::json` object.
     * @param extra Extra fields extracted from @a request
     * @return The parsed JSON RPC request.
     *
     * @details This method extracts the components of a JSON RPC request from the provided JSON object.
     */
    static jsonrpc_request parse_request(const nlohmann::json& request, nlohmann::json& extra);

    /**
     * @brief Processes a batch request.
     *
     * @param request The batch request as a `nlohmann::json` array.
     * @param extra The extra parameter to pass to the method handlers.
     * @return The response serialized into a JSON array.
     *
     * @details This method processes a batch request by invoking the method handlers for each request in the batch.
     *
     * @see dispatcher::process_request()
     */
    nlohmann::json process_batch_request(const nlohmann::json& request, const nlohmann::json& extra);

    /**
     * @brief Validates a JSON RPC request.
     *
     * @param r The JSON RPC request to validate.
     *
     * @details This method checks the validity of the JSON RPC request, ensuring that all required fields are present and correctly formatted.
     * @throws exception If the request is invalid.
     * @see exception::INVALID_REQUEST, exception::INVALID_PARAMS
     */
    static void validate_request(const jsonrpc_request& r);

    /**
     * @brief Invokes a method handler.
     *
     * @param method The name of the method to invoke.
     * @param params The parameters for the method.
     * @param extra The extra parameter to pass to the method handlers.
     * @return The result of the method invocation serialized into a JSON object.
     *
     * @details This method finds the handler for the specified method and invokes it with the provided parameters.
     * @throws exception If the method is not found or the invocation fails.
     * @see exception::METHOD_NOT_FOUND
     */
    nlohmann::json invoke(const std::string& method, const nlohmann::json& params, const nlohmann::json& extra);
};

}  // namespace wwa::json_rpc

#endif /* FB656817_7041_48D5_80B2_347168163158 */
