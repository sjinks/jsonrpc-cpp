#ifndef FB656817_7041_48D5_80B2_347168163158
#define FB656817_7041_48D5_80B2_347168163158

/**
 * @file
 * @brief Contains the private implementation details of the JSON RPC dispatcher class.
 * @internal
 */

#include <string>
#include <unordered_map>
#include <utility>

#include "dispatcher.h"

namespace wwa::json_rpc {

/**
 * @brief Private implementation of the JSON RPC dispatcher class.
 * @internal
 *
 * This class contains the private members and methods used by the `dispatcher` class to manage method handlers.
 */
class dispatcher_private {
public:
    /**
     * @brief Adds a method handler.
     *
     * @param method The name of the method.
     * @param handler The handler function.
     *
     * @details This method registers a handler function for a given method name.
     * The handler function will be invoked when a request for the specified method is received.
     */
    void add_handler(std::string&& method, dispatcher::handler_t&& handler)
    {
        this->m_methods.try_emplace(std::move(method), std::move(handler));
    }

    /**
     * @brief Finds a method handler.
     *
     * @param method The name of the method.
     * @return The handler function for the specified method.
     * @retval nullptr Handler not found
     *
     * @details This method returns the handler function for the specified method name.
     * If no handler is found, it returns a null pointer.
     */
    dispatcher::handler_t find_handler(const std::string& method) const
    {
        if (const auto it = this->m_methods.find(method); it != this->m_methods.end()) {
            return it->second;
        }

        return nullptr;
    }

private:
    /** @brief Map of method names to handler functions. */
    std::unordered_map<std::string, dispatcher::handler_t> m_methods;
};

}  // namespace wwa::json_rpc

#endif /* FB656817_7041_48D5_80B2_347168163158 */
