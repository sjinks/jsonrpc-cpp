#ifndef AB18CB0F_2A0A_401A_A253_B793A4B1FFB4
#define AB18CB0F_2A0A_401A_A253_B793A4B1FFB4

/**
 * @file
 * @brief Utility functions for JSON RPC handling.
 */

#include <string>
#include <nlohmann/json.hpp>

#include "export.h"

namespace wwa::json_rpc {

class exception;

/**
 * @brief Checks if the provided JSON value is a valid JSON RPC request ID.
 *
 * @param id The JSON value to check.
 * @return `true` if the JSON value is a valid request ID, `false` otherwise.
 *
 * @details This function checks if the provided JSON value is a valid JSON RPC request ID.
 * According to the JSON RPC specification, a valid request ID can be a string, a number, or null.
 * Additionally, this function also considers a discarded JSON value as valid.
 *
 * @see https://www.jsonrpc.org/specification#request_object
 */
WWA_JSONRPC_EXPORT bool is_valid_request_id(const nlohmann::json& id);

/**
 * @brief Get the request id object
 *
 * @param request JSON RPC request
 * @return Request ID
 *
 * @details This function extracts the ID field from a JSON RPC request object.
 * If the `id` field is not present or is not valid (see @a is_valid_request_id()),
 * it returns a `null` JSON value.
 */
WWA_JSONRPC_EXPORT nlohmann::json get_request_id(const nlohmann::json& request);

/**
 * @brief Serializes the JSON RPC response to a string.
 *
 * @param response Response to serialize.
 * @return Response serialized to a string' empty string if `response.is_discarded()` is `true`.
 */
WWA_JSONRPC_EXPORT std::string serialize_repsonse(const nlohmann::json& response);

/**
 * @brief Checks whether @a response is an error response.
 *
 * @param response JSON RPC response.
 * @return Whether the response is an error response.
 */
WWA_JSONRPC_EXPORT bool is_error_response(const nlohmann::json& response);

/**
 * @brief Gets the error code from an error response.
 *
 * @param response JSON RPC error response.
 * @return The error code.
 */
WWA_JSONRPC_EXPORT int get_error_code(const nlohmann::json& response);

/**
 * @brief Gets the error message from an error response.
 *
 * @param response JSON RPC error response.
 * @return The error message.
 */
WWA_JSONRPC_EXPORT std::string get_error_message(const nlohmann::json& response);

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
WWA_JSONRPC_EXPORT nlohmann::json
generate_error_response(const exception& e, const nlohmann::json& id = nlohmann::json::value_t::null);

}  // namespace wwa::json_rpc

#endif /* AB18CB0F_2A0A_401A_A253_B793A4B1FFB4 */
