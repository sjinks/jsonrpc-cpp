/**
 * @file
 * @brief Implementation of the dispatcher class.
 */

#include "dispatcher.h"
#include <nlohmann/json_fwd.hpp>
#include "dispatcher_p.h"
#include "exception.h"

namespace wwa::json_rpc {

dispatcher::dispatcher() : d_ptr(std::make_unique<dispatcher_private>(this)) {}

dispatcher::~dispatcher() = default;

void dispatcher::add_internal_method(std::string_view method, handler_t&& handler)
{
    this->d_ptr->add_handler(std::string(method), std::move(handler));
}

std::string dispatcher::parse_and_process_request(const std::string& request, const nlohmann::json& extra)
{
    nlohmann::json req;
    try {
        req = nlohmann::json::parse(request);
    }
    catch (const nlohmann::json::exception& e) {
        this->on_request(nlohmann::json(nlohmann::json::value_t::discarded), extra);
        const auto json = dispatcher_private::generate_error_response(
            exception(exception::PARSE_ERROR, e.what()), nlohmann::json(nullptr)
        );

        this->on_request_processed({}, json, extra);
        return json.dump();
    }

    const auto response = this->process_request(req, extra);
    return response.is_discarded() ? std::string{} : response.dump();
}

nlohmann::json dispatcher::process_request(const nlohmann::json& request, const nlohmann::json& extra)
{
    return this->d_ptr->process_request(request, extra);
}

void dispatcher::on_request(const nlohmann::json&, const nlohmann::json&)
{
    // Do nothing
}

void dispatcher::on_method(const std::string&, const nlohmann::json&)
{
    // Do nothing
}

void dispatcher::on_request_processed(const std::string&, const nlohmann::json&, const nlohmann::json&)
{
    // Do nothing
}

bool is_error_response(const nlohmann::json& response)
{
    return response.is_object() && response.contains("error") && response["error"].is_object();
}

int get_error_code(const nlohmann::json& response)
{
    return response["error"]["code"];
}

std::string get_error_message(const nlohmann::json& response)
{
    return response["error"]["message"];
}

}  // namespace wwa::json_rpc
