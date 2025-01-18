#include "request.h"
#include "exception.h"
#include "utils.h"

/**
 * @file request.cpp
 * @brief Implements the functionality for handling JSON RPC requests.
 * 
 * This source file contains the implementation of the `jsonrpc_request` struct's methods.
 * It includes the deserialization of a JSON object into a `jsonrpc_request` structure and the validation of the JSON RPC request.
 * 
 * The file uses the nlohmann::json library for JSON handling and includes additional headers for exception handling and utility functions.
 * 
 * @see https://www.jsonrpc.org/specification#request_object
 */

namespace wwa::json_rpc {

/**
 * @brief Deserializes a JSON object into a `jsonrpc_request` structure.
 * @internal
 *
 * @param j The JSON object to deserialize.
 * @param r The `jsonrpc_request` structure to populate.
 *
 * @details This function deserializes a JSON object into a `jsonrpc_request` structure.
 * * It extracts the `jsonrpc` version, `method` name, `params`, and `id` from the JSON object.
 * * If the `params` field is not present, it defaults to an empty array.
 * * If the `params` field is an object, it is wrapped in an array.
 *
 * @note This function cannot be moved to an anonymous namespace because of Argument-Dependent Lookup (ADL).
 *
 * @see https://www.jsonrpc.org/specification#request_object
 * @see https://github.com/nlohmann/json?tab=readme-ov-file#arbitrary-types-conversions
 */
// NOLINTNEXTLINE(misc-use-anonymous-namespace) -- cannot move to an anonymous namespace because of ADL
static void from_json(const nlohmann::json& j, jsonrpc_request& r)
{
    r.params = nlohmann::json(nlohmann::json::value_t::discarded);
    r.id     = nlohmann::json(nlohmann::json::value_t::discarded);

    j.at("jsonrpc").get_to(r.jsonrpc);
    j.at("method").get_to(r.method);

    if (j.contains("params")) {
        r.params = j["params"];
    }

    if (j.contains("id")) {
        r.id = j["id"];
    }

    if (r.params.is_discarded()) {
        r.params = nlohmann::json::array();
    }
    else if (r.params.is_object()) {
        r.params = nlohmann::json::array({r.params});
    }
}

jsonrpc_request jsonrpc_request::from_json(const nlohmann::json& request)
{
    jsonrpc_request req;
    try {
        request.get_to(req);
    }
    catch (const nlohmann::json::exception& e) {
        throw exception(exception::INVALID_REQUEST, e.what());
    }

    if (req.jsonrpc != "2.0") {
        throw json_rpc::exception(json_rpc::exception::INVALID_REQUEST, json_rpc::err_not_jsonrpc_2_0_request);
    }

    if (!req.params.is_array()) {
        throw json_rpc::exception(json_rpc::exception::INVALID_PARAMS, json_rpc::err_bad_params_type);
    }

    if (req.method.empty()) {
        throw json_rpc::exception(json_rpc::exception::INVALID_REQUEST, json_rpc::err_empty_method);
    }

    if (!is_valid_request_id(req.id)) {
        throw json_rpc::exception(json_rpc::exception::INVALID_REQUEST, json_rpc::err_bad_id_type);
    }

    req.extra = request;
    req.extra.erase("jsonrpc");
    req.extra.erase("method");
    req.extra.erase("params");
    req.extra.erase("id");
    return req;
}

}  // namespace wwa::json_rpc
