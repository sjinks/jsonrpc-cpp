#ifndef E66C1505_D447_4384_BB35_55B23FF31F0A
#define E66C1505_D447_4384_BB35_55B23FF31F0A

/**
 * @file
 * @brief Defines macros for exporting and importing symbols in the JSON RPC library.
 *
 * This file contains macros that control the visibility of symbols in the JSON RPC library.
 * It handles the differences between static and dynamic linking, as well as platform-specific
 * visibility attributes for Windows and Unix-like systems.
 */

/**
 * @def WWA_JSONRPC_EXPORT
 * @brief Macro for exporting symbols when building the library dynamically or importing symbols when using the library dynamically.
 */

/**
 * @def WWA_JSONRPC_NO_EXPORT
 * @brief Macro for hiding symbols.
 */

#ifdef WWA_JSONRPC_STATIC_DEFINE
#    define WWA_JSONRPC_EXPORT
#    define WWA_JSONRPC_NO_EXPORT
#else
#    ifdef wwa_jsonrpc_EXPORTS
/* We are building this library; export */
#        if defined _WIN32 || defined __CYGWIN__
#            define WWA_JSONRPC_EXPORT __declspec(dllexport)
#            define WWA_JSONRPC_NO_EXPORT
#        else
#            define WWA_JSONRPC_EXPORT    [[gnu::visibility("default")]]
#            define WWA_JSONRPC_NO_EXPORT [[gnu::visibility("hidden")]]
#        endif
#    else
/* We are using this library; import */
#        if defined _WIN32 || defined __CYGWIN__
#            define WWA_JSONRPC_EXPORT __declspec(dllimport)
#            define WWA_JSONRPC_NO_EXPORT
#        else
#            define WWA_JSONRPC_EXPORT    [[gnu::visibility("default")]]
#            define WWA_JSONRPC_NO_EXPORT [[gnu::visibility("hidden")]]
#        endif
#    endif
#endif

#endif /* E66C1505_D447_4384_BB35_55B23FF31F0A */
