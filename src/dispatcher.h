#ifndef FAB131EA_3F90_43B6_833D_EB89DA373735
#define FAB131EA_3F90_43B6_833D_EB89DA373735

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <nlohmann/json.hpp>

#include "exception.h"
#include "export.h"
#include "traits.h"

namespace wwa::json_rpc {

class dispatcher_private;
class WWA_JSONRPC_EXPORT dispatcher {
private:
    friend class dispatcher_private;
    using handler_t = std::function<nlohmann::json(const nlohmann::json&)>;

public:
    dispatcher();
    virtual ~dispatcher();

    dispatcher(const dispatcher&)            = delete;
    dispatcher& operator=(const dispatcher&) = delete;
    dispatcher(dispatcher&&)                 = default;
    dispatcher& operator=(dispatcher&&)      = default;

    template<typename F>
    void add(std::string_view method, F&& f)
    {
        this->add(method, std::forward<F>(f), nullptr);
    }

    template<typename C, typename F>
    void add(std::string_view method, F&& f, C instance)
    {
        using traits             = details::function_traits<std::decay_t<F>>;
        using ArgsTuple          = typename traits::args_tuple;
        constexpr auto args_size = std::tuple_size<ArgsTuple>();

        auto&& closure =
            this->create_closure<C, F, ArgsTuple>(instance, std::forward<F>(f), std::make_index_sequence<args_size>{});
        this->add_internal_method(method, std::forward<decltype(closure)>(closure));
    }

    std::string parse_and_process_request(const std::string& r);
    std::string process_request(const nlohmann::json& request);

    virtual void on_request();
    virtual void on_method(const std::string& method);
    virtual void on_request_processed(const std::string& method, int code);

private:
    std::unique_ptr<dispatcher_private> d_ptr;

    void add_internal_method(std::string_view method, handler_t&& handler);

    template<typename F, typename Tuple>
    nlohmann::json invoke_function(F&& f, Tuple&& tuple) const
    {
        using ReturnType = typename details::function_traits<std::decay_t<F>>::return_type;

        if constexpr (std::is_void_v<ReturnType>) {
            std::apply(std::forward<F>(f), std::forward<Tuple>(tuple));
            // NOLINTNEXTLINE(modernize-return-braced-init-list) -- braced init will create a JSON array
            return nlohmann::json(nullptr);
        }
        else {
            return std::apply(std::forward<F>(f), std::forward<Tuple>(tuple));
        }
    }

    template<typename C, typename F, typename Args, std::size_t... Indices>
    constexpr auto
    create_closure(C inst, F&& f, std::index_sequence<Indices...>)  // NOLINT(readability-function-cognitive-complexity)
    {
        static_assert(std::is_pointer_v<C> || std::is_null_pointer_v<C>);
        return [func = std::forward<F>(f), inst, this](const nlohmann::json& params) {
            constexpr auto args_size = std::tuple_size<Args>();

            if (params.is_array()) {
                if constexpr (args_size == 1) {
                    if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<0, Args>>, nlohmann::json>) {
                        auto&& tuple_args = [inst, &params]() constexpr {
                            (void)inst;
                            if constexpr (std::is_null_pointer_v<C>) {
                                return std::make_tuple(params);
                            }
                            else {
                                return std::make_tuple(inst, params);
                            }
                        }();

                        return this->invoke_function(func, std::forward<decltype(tuple_args)>(tuple_args));
                    }
                }

                if (params.size() == std::tuple_size<Args>()) {
                    auto&& tuple_args = [inst, &params]() constexpr {
                        (void)inst;
                        (void)params;
                        try {
                            if constexpr (std::is_null_pointer_v<C>) {
                                return std::make_tuple(
                                    params[Indices].get<std::decay_t<std::tuple_element_t<Indices, Args>>>()...
                                );
                            }
                            else {
                                return std::make_tuple(
                                    inst, params[Indices].get<std::decay_t<std::tuple_element_t<Indices, Args>>>()...
                                );
                            }
                        }
                        catch (const nlohmann::json::exception& e) {
                            throw exception(exception::INVALID_PARAMS, e.what());
                        }
                    }();

                    return this->invoke_function(func, std::forward<decltype(tuple_args)>(tuple_args));
                }
            }

            throw exception(exception::INVALID_PARAMS, err_invalid_params_passed_to_method);
        };
    }
};

}  // namespace wwa::json_rpc

#endif /* FAB131EA_3F90_43B6_833D_EB89DA373735 */
