#ifndef C8EEBB64_DA22_4649_BD6D_8BF0AE756F87
#define C8EEBB64_DA22_4649_BD6D_8BF0AE756F87

/**
 * @file
 * @brief Defines the structure and functionality for handling JSON RPC requests.
 */

#include <string>
#include <nlohmann/json.hpp>

namespace wwa::json_rpc {

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
    nlohmann::json extra;   ///< Extra fields from the JSON RPC request.

    /**
     * @brief Parses and validates a JSON RPC request.
     *
     * @param request The JSON RPC request as a `nlohmann::json` object.
     * @return The parsed JSON RPC request.
     *
     * @details This method extracts the components of a JSON RPC request from the provided JSON object and validates the request,
     * ensuring that all required fields are present and correctly formatted.
     * @throws exception If the request is invalid.
     * @see exception::INVALID_REQUEST, exception::INVALID_PARAMS
     */
    static jsonrpc_request from_json(const nlohmann::json& request);
};

}  // namespace wwa::json_rpc

#endif /* C8EEBB64_DA22_4649_BD6D_8BF0AE756F87 */
