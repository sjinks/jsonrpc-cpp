/**
 * @file
 * @brief Implementation of the JSON RPC exception class.
 *
 * This file contains the implementation of destructor for the exception classes.
 * The destructors are defined here to avoid vtables in every translation unit that includes the header.
 */

#include "exception.h"

wwa::json_rpc::exception::~exception()                                   = default;
wwa::json_rpc::method_not_found_exception::~method_not_found_exception() = default;
