#include "utils.h"

namespace wwa::json_rpc {

bool is_valid_request_id(const nlohmann::json& id)
{
    return id.is_string() || id.is_number() || id.is_null() || id.is_discarded();
}

nlohmann::json get_request_id(const nlohmann::json& request)
{
    auto id = request.contains("id") ? request["id"] : nlohmann::json(nullptr);
    return is_valid_request_id(id) ? id : nlohmann::json(nullptr);
}

std::string serialize_repsonse(const nlohmann::json& response)
{
    return response.is_discarded() ? std::string{} : response.dump();
}

bool is_error_response(const nlohmann::json& response)
{
    return response.is_object() && response.contains("error") && response["error"].is_object();
}

int get_error_code(const nlohmann::json& response)
{
    return response.at("error").value("code", 0);
}

std::string get_error_message(const nlohmann::json& response)
{
    return response.at("error").value("message", "");
}

nlohmann::json generate_error_response(const exception& e, const nlohmann::json& id)
{
    // clang-format off
    return {
        {"jsonrpc", "2.0"},
        {"error", e.to_json()},
        {"id", id}
    };
    // clang-format on
}

}  // namespace wwa::json_rpc
