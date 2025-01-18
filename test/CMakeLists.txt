add_executable(
    test_jsonrpc
    base.cpp
    test_error_handling.cpp
    test_exception.cpp
    test_extra_param.cpp
    test_invocation.cpp
    test_notifications.cpp
    test_utils.cpp
)

target_compile_features(test_jsonrpc PRIVATE cxx_std_20)
target_link_libraries(test_jsonrpc PRIVATE ${PROJECT_NAME} GTest::gtest_main)

if(ENABLE_MAINTAINER_MODE)
    target_compile_options(test_jsonrpc PRIVATE ${CMAKE_CXX_FLAGS_MM})
    if(CMAKE_COMPILER_IS_CLANG)
        target_compile_options(test_jsonrpc PRIVATE -Wno-weak-vtables -Wno-global-constructors)
    endif()
endif()

if(NOT CMAKE_CROSSCOMPILING)
    include(GoogleTest)
    gtest_discover_tests(test_jsonrpc)
endif()
