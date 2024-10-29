#ifndef CC75354D_5C03_4B34_B773_96A9E6189611
#define CC75354D_5C03_4B34_B773_96A9E6189611

/**
 * @file exception.h
 * @brief Contains the definition of the JSON RPC Exception class.
 * @see https://www.jsonrpc.org/specification#error_object
 */

#include <exception>
#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

#include "export.h"

namespace wwa::json_rpc {

/**
 * @defgroup error_message Error Messages
 * @brief Error messages used by the library. These constants can be useful in unit tests.
 * @{
 */

/**
 * @brief Error message for when the request is not a JSON-RPC 2.0 request.
 * @see exception::INVALID_REQUEST
 * @see https://www.jsonrpc.org/specification#request_object
 * @details > `jsonrpc`: A String specifying the version of the JSON-RPC protocol. MUST be exactly "2.0".
 */
static constexpr std::string_view err_not_jsonrpc_2_0_request = "Not a JSON-RPC 2.0 request";

/**
 * @brief Error message for when the parameters passed to the method are not correct.
 * @see exception::INVALID_PARAMS
 */
static constexpr std::string_view err_invalid_params_passed_to_method = "Invalid parameters passed to method";

/**
 * @brief Error message for when the method is not found.
 * @see exception::METHOD_NOT_FOUND
 */
static constexpr std::string_view err_method_not_found = "Method not found";

/**
 * @brief Error message for when the method is empty.
 * @see exception::INVALID_REQUEST
 */
static constexpr std::string_view err_empty_method = "Method cannot be empty";

/**
 * @brief Error message for when the parameters are not an array or an object.
 * @see exception::INVALID_PARAMS
 */
static constexpr std::string_view err_bad_params_type = "Parameters must be either an array or an object or omitted";

/**
 * @brief Error message for when the ID is not a number, a string, or null.
 * @see exception::INVALID_REQUEST
 * @see https://www.jsonrpc.org/specification#request_object
 * @details > An identifier established by the Client that MUST contain a String, Number, or NULL value if included.
 */
static constexpr std::string_view err_bad_id_type = "ID must be either a number, a string, or null";

/**
 * @brief Error message for when the batch request is empty.
 * @see exception::INVALID_REQUEST
 * @see https://www.jsonrpc.org/specification#batch
 */
static constexpr std::string_view err_empty_batch = "Empty batch request";
/** @} */

/**
 * @brief JSON RPC Exception class.
 *
 * This class represents an exception that can occur during the processing of JSON RPC requests.
 * It includes an error code, a message, and optional additional data.
 */
class WWA_JSONRPC_EXPORT exception : public std::exception {
public:
    /**
     * @defgroup error_codes Error Codes
     * @brief Error codes defined by the [JSON-RPC 2.0 specification](https://www.jsonrpc.org/specification#request_object).
     * @see https://www.jsonrpc.org/specification#error_object
     * @{
     */

    /**
     * @brief Invalid JSON was received by the server.
     *
     * An error occurred on the server while parsing the JSON text.
     */
    static constexpr int PARSE_ERROR = -32700;

    /**
     * @brief The JSON sent is not a valid Request object.
     */
    static constexpr int INVALID_REQUEST = -32600;

    /**
     * @brief The method does not exist or is not available.
     */
    static constexpr int METHOD_NOT_FOUND = -32601;

    /**
     * @brief Invalid method parameter(s).
     */
    static constexpr int INVALID_PARAMS = -32602;

    /**
     * @brief Internal JSON-RPC error.
     */
    static constexpr int INTERNAL_ERROR = -32603;
    /** @} */

    /**
     * @brief Construct a new exception object with additional data.
     *
     * @tparam T Type of the @a data. Must be [convertible to `nlohmann::json`](https://github.com/nlohmann/json?tab=readme-ov-file#arbitrary-types-conversions).
     * @param code Indicates the error type that occurred.
     * @param message Provides a short description of the error. The message SHOULD be limited to a concise single sentence.
     * @param data Additional information about the error.
     *
     * @see https://www.jsonrpc.org/specification#error_object
     */
    template<typename T>
    exception(int code, std::string_view message, const T& data) : m_message(message), m_data(data), m_code(code)
    {}

    /**
     * @brief Construct a new exception object.
     *
     * @param code Indicates the error type that occurred.
     * @param message Provides a short description of the error. The message SHOULD be limited to a concise single sentence.
     *
     * @see https://www.jsonrpc.org/specification#error_object
     */
    exception(int code, std::string_view message) : m_message(message), m_code(code) {}

    /**
     * @brief Default copy constructor.
     */
    exception(const exception&) = default;

    /**
     * @brief Default move constructor.
     */
    exception(exception&&) = default;

    /**
     * @brief Default copy assignment operator.
     *
     * @param rhs Right-hand side of the assignment.
     * @return Reference to this object.
     */
    exception& operator=(const exception& rhs) = default;

    /**
     * @brief Default move assignment operator.
     *
     * @param rhs Right-hand side of the assignment.
     * @return Reference to this object.
     */
    exception& operator=(exception&& rhs) = default;

    /**
     * @brief Default destructor
     */
    ~exception() override;

    /**
     * @brief Returns the error code.
     *
     * @return Error code.
     */
    [[nodiscard]] int code() const noexcept { return this->m_code; }

    /**
     * @brief Returns the error message.
     *
     * @return Error message.
     */
    [[nodiscard]] const std::string& message() const noexcept { return this->m_message; }

    /**
     * @brief Returns custom data associated with the error.
     *
     * @return Custom data in JSON format.
     */
    [[nodiscard]] const nlohmann::json& data() const noexcept { return this->m_data; }

    /**
     * @brief Returns the error message.
     *
     * @see https://en.cppreference.com/w/cpp/error/exception/what
     * @return Pointer to a null-terminated string with explanatory information.
     *
     * @see message()
     */
    [[nodiscard]] const char* what() const noexcept override { return this->m_message.c_str(); }

    /**
     * @brief Returns the error message as an Error Object.
     *
     * @see https://www.jsonrpc.org/specification#error_object
     * @return Error Object as JSON.
     */
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
    std::string m_message;  ///< Error message.
    nlohmann::json m_data;  ///< Custom data associated with the error.
    int m_code;             ///< Error code.
};

}  // namespace wwa::json_rpc

#endif /* CC75354D_5C03_4B34_B773_96A9E6189611 */
