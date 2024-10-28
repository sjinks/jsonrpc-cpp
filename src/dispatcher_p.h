#ifndef FB656817_7041_48D5_80B2_347168163158
#define FB656817_7041_48D5_80B2_347168163158

#include <functional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include "dispatcher.h"

struct jsonrpc_request;

namespace wwa::json_rpc {

class exception;

struct hasher {
    using is_transparent = void;

    std::size_t operator()(std::string_view s) const noexcept
    {
        const std::hash<std::string_view> h;
        return h(s);
    }
};

struct jsonrpc_request {
    std::string jsonrpc;
    std::string method;
    nlohmann::json params;
    nlohmann::json id;
};

class dispatcher_private {
public:
    explicit dispatcher_private(dispatcher* q) : q_ptr(q) {}
    void add_handler(std::string_view method, dispatcher::handler_t&& handler);
    nlohmann::json process_request(const nlohmann::json& request);
    static nlohmann::json generate_error_response(const exception& e, const nlohmann::json& id);

private:
    dispatcher* const q_ptr;
    std::unordered_map<std::string, dispatcher::handler_t, hasher, std::equal_to<>> m_methods;
    std::shared_mutex m_methods_mutex;

    static jsonrpc_request parse_request(const nlohmann::json& request);
    static void validate_request(const jsonrpc_request& r);
    dispatcher::handler_t get_method(const std::string& method);
    nlohmann::json invoke(const std::string& method, const nlohmann::json& params);
};

}  // namespace wwa::json_rpc

#endif /* FB656817_7041_48D5_80B2_347168163158 */
