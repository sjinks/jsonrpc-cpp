#include "dispatcher.h"
#include "dispatcher_p.h"
#include "exception.h"

namespace wwa::json_rpc {

dispatcher::dispatcher() : d_ptr(std::make_unique<dispatcher_private>(this)) {}

dispatcher::~dispatcher() = default;

void dispatcher::add_internal_method(std::string_view method, handler_t&& handler)
{
    this->d_ptr->add_handler(std::string(method), std::move(handler));
}

std::string dispatcher::parse_and_process_request(const std::string& r)
{
    nlohmann::json request;
    try {
        request = nlohmann::json::parse(r);
    }
    catch (const nlohmann::json::exception& e) {
        this->on_request();
        const auto json = dispatcher_private::generate_error_response(
            exception(exception::PARSE_ERROR, e.what()), nlohmann::json(nullptr)
        );

        this->on_request_processed({}, exception::PARSE_ERROR);
        return json.dump();
    }

    return this->process_request(request);
}

std::string dispatcher::process_request(const nlohmann::json& request)
{
    const auto json = this->d_ptr->process_request(request);
    return json.is_discarded() ? std::string{} : json.dump();
}

void dispatcher::on_request()
{
    // Do nothing
}

void dispatcher::on_method(const std::string&)
{
    // Do nothing
}

void dispatcher::on_request_processed(const std::string&, int)
{
    // Do nothing
}

}  // namespace wwa::json_rpc
