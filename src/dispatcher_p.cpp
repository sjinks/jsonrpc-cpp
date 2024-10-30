#include "dispatcher_p.h"
#include "dispatcher.h"
#include "exception.h"

/**
 * @file
 * @brief Implementation of the private members and methods of the JSON RPC dispatcher class.
 * @internal
 *
 * This file contains the definitions of the private members and methods used by the `dispatcher` class to manage method handlers and process requests.
 * It includes functions for parsing, validating, and invoking JSON RPC requests, as well as generating error responses.
 */

namespace {

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
 * @internal
 */
bool is_valid_request_id(const nlohmann::json& id)
{
    return id.is_string() || id.is_number() || id.is_null() || id.is_discarded();
}

}  // namespace

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
// NOLINT(misc-use-anonymous-namespace) -- cannot move to an anonymous namespace because of ADL
static void from_json(const nlohmann::json& j, jsonrpc_request& r)
{
    r.params = nlohmann::json(nlohmann::json::value_t::discarded);
    r.id     = nlohmann::json(nlohmann::json::value_t::discarded);

    j.at("jsonrpc").get_to(r.jsonrpc);
    j.at("method").get_to(r.method);

    if (j.contains("params")) {
        j.at("params").get_to(r.params);
    }

    if (j.contains("id")) {
        j.at("id").get_to(r.id);
    }

    if (r.params.is_discarded()) {
        r.params = nlohmann::json::array();
    }
    else if (r.params.is_object()) {
        r.params = nlohmann::json::array({r.params});
    }
}

void dispatcher_private::add_handler(std::string&& method, dispatcher::handler_t&& handler)
{
    this->m_methods.try_emplace(std::move(method), std::move(handler));
}

jsonrpc_request dispatcher_private::parse_request(const nlohmann::json& request, nlohmann::json& extra)
{
    try {
        auto req = request.get<jsonrpc_request>();
        extra    = request;
        extra.erase("jsonrpc");
        extra.erase("method");
        extra.erase("params");
        extra.erase("id");
        return req;
    }
    catch (const nlohmann::json::exception& e) {
        throw exception(exception::INVALID_REQUEST, e.what());
    }
}

// NOLINTNEXTLINE(misc-no-recursion) -- false positive, process_request() will never be called with an object
nlohmann::json dispatcher_private::process_batch_request(const nlohmann::json& request, const nlohmann::json& extra)
{
    if (request.empty()) {
        this->q_ptr->on_request();
        this->q_ptr->on_request_processed({}, exception::INVALID_REQUEST);
        return dispatcher_private::generate_error_response(
            exception(exception::INVALID_REQUEST, err_empty_batch), nlohmann::json(nullptr)
        );
    }

    auto response = nlohmann::json::array();
    for (const auto& req : request) {
        if (!req.is_object()) {
            this->q_ptr->on_request();
            const auto r = dispatcher_private::generate_error_response(
                exception(exception::INVALID_REQUEST, err_not_jsonrpc_2_0_request), nlohmann::json(nullptr)
            );

            response.push_back(r);
            this->q_ptr->on_request_processed({}, exception::INVALID_REQUEST);
        }
        else if (const auto res = this->process_request(req, extra); !res.is_discarded()) {
            response.push_back(res);
        }
    }

    return response.empty() ? nlohmann::json(nlohmann::json::value_t::discarded) : response;
}

// NOLINTNEXTLINE(misc-no-recursion) -- there is one level of recursion for batch requests
nlohmann::json dispatcher_private::process_request(const nlohmann::json& request, const nlohmann::json& extra)
{
    if (request.is_array()) {
        return this->process_batch_request(request, extra);
    }

    this->q_ptr->on_request();
    auto request_id = request.contains("id") ? request["id"] : nlohmann::json(nullptr);
    if (!is_valid_request_id(request_id)) {
        request_id = nlohmann::json(nullptr);
    }

    std::string method;
    try {
        nlohmann::json extra_fields;
        auto req = dispatcher_private::parse_request(request, extra_fields);
        dispatcher_private::validate_request(req);
        method     = req.method;
        request_id = req.id;

        nlohmann::json extra_data = extra;
        if (extra.is_object() && !extra_fields.empty()) {
            extra_data["extra"] = extra_fields;
        }

        const auto res = this->invoke(method, req.params, extra_data);
        if (!req.id.is_discarded()) {
            return nlohmann::json({{"jsonrpc", "2.0"}, {"result", res}, {"id", req.id}});
        }

        // NOLINTNEXTLINE(modernize-return-braced-init-list) -- braced init will create a JSON array
        return nlohmann::json(nlohmann::json::value_t::discarded);
    }
    catch (const exception& e) {
        this->q_ptr->on_request_processed(method, e.code());
        return request_id.is_discarded() ? nlohmann::json(nlohmann::json::value_t::discarded)
                                         : dispatcher_private::generate_error_response(e, request_id);
    }
    catch (const std::exception& e) {
        this->q_ptr->on_request_processed(method, exception::INTERNAL_ERROR);
        return request_id.is_discarded() ? nlohmann::json(nlohmann::json::value_t::discarded)
                                         : dispatcher_private::generate_error_response(
                                               exception(exception::INTERNAL_ERROR, e.what()), request_id
                                           );
    }
}

void dispatcher_private::validate_request(const jsonrpc_request& r)
{
    if (r.jsonrpc != "2.0") {
        throw json_rpc::exception(json_rpc::exception::INVALID_REQUEST, json_rpc::err_not_jsonrpc_2_0_request);
    }

    if (!r.params.is_array()) {
        throw json_rpc::exception(json_rpc::exception::INVALID_PARAMS, json_rpc::err_bad_params_type);
    }

    if (r.method.empty()) {
        throw json_rpc::exception(json_rpc::exception::INVALID_REQUEST, json_rpc::err_empty_method);
    }

    if (!is_valid_request_id(r.id)) {
        throw json_rpc::exception(json_rpc::exception::INVALID_REQUEST, json_rpc::err_bad_id_type);
    }
}

nlohmann::json dispatcher_private::generate_error_response(const exception& e, const nlohmann::json& id)
{
    return nlohmann::json({{"jsonrpc", "2.0"}, {"error", e.to_json()}, {"id", id}});
}

nlohmann::json
dispatcher_private::invoke(const std::string& method, const nlohmann::json& params, const nlohmann::json& extra)
{
    if (const auto it = this->m_methods.find(method); it != this->m_methods.end()) {
        this->q_ptr->on_method(method);
        const auto response = it->second(extra, params);
        this->q_ptr->on_request_processed(method, 0);
        return response;
    }

    throw exception(exception::METHOD_NOT_FOUND, err_method_not_found);
}

}  // namespace wwa::json_rpc
