/**
 * @file
 * @brief Implementation of the dispatcher class.
 */

#include "dispatcher.h"
#include "dispatcher_p.h"
#include "exception.h"
#include "request.h"
#include "utils.h"

namespace wwa::json_rpc {

dispatcher::dispatcher() : d_ptr(std::make_unique<dispatcher_private>()) {}

dispatcher::~dispatcher() = default;

void dispatcher::add_internal_method(std::string_view method, handler_t&& handler)
{
    this->d_ptr->add_handler(std::string(method), std::move(handler));
}

nlohmann::json dispatcher::process_request(const nlohmann::json& request, const std::any& data)
{
    const auto unique_id = dispatcher_private::get_and_increment_counter();
    if (request.is_array()) {
        return this->process_batch_request(request, data, unique_id);
    }

    return this->do_process_request(request, data, false, unique_id);
}

nlohmann::json
dispatcher::do_process_request(const nlohmann::json& request, const std::any& data, bool, std::uint64_t unique_id)
{
    nlohmann::json discarded = nlohmann::json::value_t::discarded;

    auto request_id   = get_request_id(request);
    bool is_discarded = false;
    try {
        const auto req = jsonrpc_request::from_json(request);
        is_discarded   = req.id.is_discarded();

        const dispatcher::context_t ctx = std::make_pair(data, req.extra);
        const auto res                  = this->invoke(req.method, req.params, ctx, unique_id);
        if (!request_id.is_null()) {
            // clang-format off
            return {
                {"jsonrpc", "2.0"},
                {"result", res},
                {"id", req.id}
            };
            // clang-format on
        }

        return discarded;
    }
    catch (const std::exception& e) {
        this->request_failed(request_id, &e, false, unique_id);
        const auto* eptr = dynamic_cast<const exception*>(&e);
        const auto ex    = eptr != nullptr ? *eptr : exception(exception::INTERNAL_ERROR, e.what());
        return is_discarded ? discarded : generate_error_response(ex, request_id);
    }
}

nlohmann::json
dispatcher::process_batch_request(const nlohmann::json& request, const std::any& data, std::uint64_t unique_id)
{
    if (request.empty()) {
        const exception e(exception::INVALID_REQUEST, err_empty_batch);
        this->request_failed(nullptr, &e, true, unique_id);
        return generate_error_response(e, nlohmann::json(nullptr));
    }

    auto response = nlohmann::json::array();
    for (const auto& req : request) {
        if (!req.is_object()) {
            const exception e(exception::INVALID_REQUEST, err_not_jsonrpc_2_0_request);
            this->request_failed(nullptr, &e, false, unique_id);

            response.push_back(generate_error_response(e, nlohmann::json(nullptr)));
        }
        else if (const auto res =
                     this->do_process_request(req, data, true, dispatcher_private::get_and_increment_counter());
                 !res.is_discarded())
        {
            response.push_back(res);
        }
    }

    return response.empty() ? nlohmann::json(nlohmann::json::value_t::discarded) : response;
}

void dispatcher::request_parsed(const jsonrpc_request&, const std::any&, std::uint64_t)
{
    // Do nothing
}

nlohmann::json dispatcher::invoke(
    const std::string& method, const nlohmann::json& params, const dispatcher::context_t& ctx, std::uint64_t
)
{
    if (const auto handler = this->d_ptr->find_handler(method); handler != nullptr) {
        return handler(ctx, params);
    }

    throw method_not_found_exception();
}

void dispatcher::request_failed(const nlohmann::json&, const std::exception*, bool, std::uint64_t)
{
    // Do nothing
}

}  // namespace wwa::json_rpc
