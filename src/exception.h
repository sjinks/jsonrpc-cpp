#ifndef CC75354D_5C03_4B34_B773_96A9E6189611
#define CC75354D_5C03_4B34_B773_96A9E6189611

#include <exception>
#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

#include "export.h"

namespace wwa::json_rpc {

static constexpr std::string_view err_not_jsonrpc_2_0_request         = "Not a JSON-RPC 2.0 request";
static constexpr std::string_view err_invalid_params_passed_to_method = "Invalid parameters passed to method";
static constexpr std::string_view err_method_not_found                = "Method not found";
static constexpr std::string_view err_empty_method                    = "Method cannot be empty";
static constexpr std::string_view err_bad_params_type = "Parameters must be either an array or an object or omitted";
static constexpr std::string_view err_bad_id_type     = "ID must be either a number, a string, or null";
static constexpr std::string_view err_empty_batch     = "Empty batch request";

class WWA_JSONRPC_EXPORT exception : public std::exception {
public:
    static constexpr int PARSE_ERROR      = -32700;
    static constexpr int INVALID_REQUEST  = -32600;
    static constexpr int METHOD_NOT_FOUND = -32601;
    static constexpr int INVALID_PARAMS   = -32602;
    static constexpr int INTERNAL_ERROR   = -32603;

    template<typename T>
    exception(int code, std::string_view message, const T& data) : m_message(message), m_data(data), m_code(code)
    {}

    exception(int code, std::string_view message) : m_message(message), m_code(code) {}

    exception(const exception&)            = default;
    exception(exception&&)                 = default;
    exception& operator=(const exception&) = default;
    exception& operator=(exception&&)      = default;

    ~exception() override;

    [[nodiscard]] int code() const noexcept { return this->m_code; }
    [[nodiscard]] const std::string& message() const noexcept { return this->m_message; }
    [[nodiscard]] const nlohmann::json& data() const noexcept { return this->m_data; }

    [[nodiscard]] const char* what() const noexcept override { return this->m_message.c_str(); }

    [[nodiscard]] nlohmann::json to_json() const
    {
        nlohmann::json j{
            {"code", this->m_code},
            {"message", this->m_message},
        };

        if (!this->m_data.is_null()) {
            j["data"] = this->m_data;
        }

        return j;
    }

private:
    std::string m_message;
    nlohmann::json m_data;
    int m_code;
};

}  // namespace wwa::json_rpc

#endif /* CC75354D_5C03_4B34_B773_96A9E6189611 */
