#ifndef FB656817_7041_48D5_80B2_347168163158
#define FB656817_7041_48D5_80B2_347168163158

#include <functional>
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
    void add_handler(std::string&& method, dispatcher::handler_t&& handler);
    nlohmann::json process_request(const nlohmann::json& request);
    static nlohmann::json generate_error_response(const exception& e, const nlohmann::json& id);

private:
    dispatcher* q_ptr;
    std::unordered_map<std::string, dispatcher::handler_t, hasher, std::equal_to<>> m_methods;

    static jsonrpc_request parse_request(const nlohmann::json& request);
    static void validate_request(const jsonrpc_request& r);
    nlohmann::json invoke(const std::string& method, const nlohmann::json& params);
};

}  // namespace wwa::json_rpc

#endif /* FB656817_7041_48D5_80B2_347168163158 */
