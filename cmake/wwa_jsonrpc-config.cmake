get_filename_component(JSONRPC_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

list(APPEND CMAKE_MODULE_PATH ${JSONRPC_CMAKE_DIR})

include(CMakeFindDependencyMacro)
find_dependency(nlohmann_json)

if(NOT TARGET wwa_jsonrpc)
    include("${JSONRPC_CMAKE_DIR}/wwa_jsonrpc-target.cmake")
    add_library(wwa::jsonrpc ALIAS wwa_jsonrpc)
endif()
