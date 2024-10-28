#ifndef E66C1505_D447_4384_BB35_55B23FF31F0A
#define E66C1505_D447_4384_BB35_55B23FF31F0A

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
