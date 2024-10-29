/**
 * @file
 * @brief Implementation of the JSON RPC exception class.
 *
 * This file contains the implementation of the destructor for the `exception` class.
 * The destructor is defined here to avoid vtables in every translation unit that includes the header.
 */

#include "exception.h"

wwa::json_rpc::exception::~exception() = default;
