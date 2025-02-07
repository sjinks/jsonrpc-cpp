cmake_minimum_required(VERSION 3.25)

set(EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_DOCS "Build documentation" ON)
option(ENABLE_MAINTAINER_MODE "Enable maintainer mode" OFF)

project(
    wwa_jsonrpc
    VERSION 3.1.1
    DESCRIPTION "JSON-RPC 2.0 library"
    HOMEPAGE_URL "https://sjinks.github.io/jsonrpc-cpp/"
    LANGUAGES CXX
)

if(CMAKE_CONFIGURATION_TYPES)
    list(APPEND CMAKE_CONFIGURATION_TYPES "Coverage" "ASAN" "LSAN" "UBSAN")
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
string(TOLOWER "${CMAKE_CONFIGURATION_TYPES}" CMAKE_CONFIGURATION_TYPES_LOWER)

string(REGEX MATCH "Clang" CMAKE_COMPILER_IS_CLANG "${CMAKE_CXX_COMPILER_ID}")
string(REGEX MATCH "GNU" CMAKE_COMPILER_IS_GNU "${CMAKE_CXX_COMPILER_ID}")

include(FetchContent)

set(CMAKE_CXX_FLAGS_MM "")
if(ENABLE_MAINTAINER_MODE)
    if(CMAKE_COMPILER_IS_CLANG)
        set(CMAKE_CXX_FLAGS_MM -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++17-compat -Wno-c++20-compat -Wno-padded -Werror)
    elseif(CMAKE_COMPILER_IS_GNU)
        set(CMAKE_CXX_FLAGS_MM -Wall -Wextra -Werror -pedantic)
    endif()
endif()

if(BUILD_TESTS)
    include(FindGTest)
    find_package(GTest CONFIG)
    if(NOT TARGET GTest::gtest)
        message(STATUS "Google Test not found, fetching it from GitHub")
        # renovate: datasource=github-tags depName=google/googletest
        set(GTEST_VERSION "v1.16.0")
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest
            GIT_TAG "${GTEST_VERSION}"
            GIT_SHALLOW ON
        )

        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

if(BUILD_DOCS)
    include(FindDoxygen)
    find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(WARNING "Doxygen not found, documentation will not be built")
        set(BUILD_DOCS OFF)
    else()
        if(TARGET Doxygen::dot)
            set(HAVE_DOT "YES")
        else()
            set(HAVE_DOT "NO")
        endif()
        configure_file("${CMAKE_SOURCE_DIR}/cmake/Doxyfile.in" "${CMAKE_BINARY_DIR}/Doxyfile" @ONLY)
        add_custom_target(
            doxygen
            COMMAND Doxygen::doxygen "${CMAKE_BINARY_DIR}/Doxyfile"
            COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_BINARY_DIR}/docs/html/" "${CMAKE_SOURCE_DIR}/apidocs"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            DEPENDS ${PROJECT_NAME}
            VERBATIM
        )
    endif()
endif()

if(CMAKE_COMPILER_IS_GNU OR CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS_ASAN "-O1 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    set(CMAKE_CXX_FLAGS_LSAN "-O1 -g -fsanitize=leak -fno-omit-frame-pointer -fno-optimize-sibling-calls")

    if(CMAKE_COMPILER_IS_GNU)
        set(CMAKE_CXX_FLAGS_COVERAGE "-Og -g --coverage -fprofile-abs-path")
        set(CMAKE_CXX_FLAGS_UBSAN "-O1 -g -fsanitize=undefined -fsanitize=float-divide-by-zero -fno-omit-frame-pointer")
    elseif(CMAKE_COMPILER_IS_CLANG)
        set(CMAKE_CXX_FLAGS_COVERAGE "-O1 -g --coverage")
        set(CMAKE_CXX_FLAGS_UBSAN "-O1 -g -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=integer -fsanitize=implicit-conversion -fsanitize=local-bounds -fsanitize=nullability -fno-omit-frame-pointer")
    endif()
endif()

find_package(nlohmann_json 3.11.3 QUIET)
set(export_nlohmann_json OFF)
if(NOT TARGET nlohmann_json::nlohmann_json)
    message(STATUS "nlohmann_json not found, fetching it from GitHub")

    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_ImplicitConversions OFF CACHE INTERNAL "")
    set(JSON_Install ON CACHE INTERNAL "")
    set(JSON_MultipleHeaders OFF CACHE INTERNAL "")
    set(JSON_SystemInclude ON CACHE INTERNAL "")

    # renovate: datasource=github-tags depName=nlohmann/json
    set(JSON_VERSION "v3.11.3")
    FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json GIT_TAG "${JSON_VERSION}" GIT_SHALLOW ON)
    FetchContent_MakeAvailable(json)
    set(export_nlohmann_json ON)
endif()

add_library(${PROJECT_NAME})
target_sources(
    ${PROJECT_NAME}
    PRIVATE
        src/exception.cpp
        src/dispatcher.cpp
        src/request.cpp
        src/utils.cpp
    PUBLIC
        FILE_SET HEADERS
        TYPE HEADERS
        BASE_DIRS src
        FILES
            src/dispatcher.h
            src/exception.h
            src/export.h
            src/details.h
            src/request.h
            src/utils.h
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        INTERFACE_COMPILE_FEATURES cxx_std_20
        POSITION_INDEPENDENT_CODE ON
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WWA_JSONRPC_STATIC_DEFINE)
endif()

if(ENABLE_MAINTAINER_MODE)
    target_compile_options(${PROJECT_NAME} PRIVATE ${CMAKE_CXX_FLAGS_MM})
endif()

if(BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()

find_program(CLANG_FORMAT NAMES clang-format)
find_program(CLANG_TIDY NAMES clang-tidy)

if(CLANG_FORMAT OR CLANG_TIDY)
    file(GLOB_RECURSE ALL_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} LIST_DIRECTORIES OFF src/*.cpp test/*.cpp)
    file(GLOB_RECURSE ALL_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} LIST_DIRECTORIES OFF src/*.h test/*.h)

    if(CLANG_FORMAT)
        add_custom_target(
            format
            COMMAND ${CLANG_FORMAT} --Wno-error=unknown -i -style=file ${ALL_SOURCE_FILES} ${ALL_HEADER_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()

    if(CLANG_TIDY)
        add_custom_target(
            tidy
            COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${ALL_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endif()

set(ENABLE_COVERAGE OFF)
if("coverage" IN_LIST CMAKE_CONFIGURATION_TYPES_LOWER OR "coverage" STREQUAL CMAKE_BUILD_TYPE_LOWER)
    if(CMAKE_COMPILER_IS_GNU OR CMAKE_COMPILER_IS_CLANG)
        find_program(GCOVR gcovr)
        if (GCOVR)
            if(CMAKE_COMPILER_IS_GNU)
                find_program(GCOV gcov)
                set(GCOV_TOOL_NAME gcov)
                set(GCOV_TOOL gcov)
            elseif(CMAKE_COMPILER_IS_CLANG)
                find_program(GCOV llvm-cov)
                set(GCOV_TOOL_NAME llvm-cov)
                set(GCOV_TOOL llvm-cov gcov)
            endif()

            if(GCOV)
                set(ENABLE_COVERAGE ON)
            else()
                message(WARNING "${GCOV_TOOL_NAME} not found, coverage report will not be generated")
            endif()
        else()
            message(WARNING "gcovr not found, coverage report will not be generated")
        endif()
    endif()
endif()

if(ENABLE_COVERAGE)
    add_custom_target(
        coverage
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${PROJECT_BINARY_DIR}/coverage"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/coverage"
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> -T test --output-on-failure
        COMMAND
            gcovr -f "${PROJECT_SOURCE_DIR}/src/" -r "${PROJECT_SOURCE_DIR}"
                --html-details -o "${PROJECT_BINARY_DIR}/coverage/index.html"
                --exclude-noncode-lines --exclude-throw-branches --exclude-unreachable-branches --decisions
                --gcov-executable="${GCOV_TOOL}"
                --print-summary
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        USES_TERMINAL
    )
    add_dependencies(coverage test_jsonrpc)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
set(targets ${PROJECT_NAME})
if(export_nlohmann_json)
    list(APPEND targets nlohmann_json)
endif()
install(
    TARGETS ${targets}
    EXPORT ${PROJECT_NAME}-target
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/wwa/jsonrpc"
)

install(
    EXPORT ${PROJECT_NAME}-target
    FILE ${PROJECT_NAME}-target.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    COPYONLY
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_file("${CMAKE_SOURCE_DIR}/cmake/pkg-config.pc.in" "${PROJECT_NAME}.pc" @ONLY)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
