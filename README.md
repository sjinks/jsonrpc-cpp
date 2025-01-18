# JSON RPC

[![Build and Test](https://github.com/sjinks/jsonrpc-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/sjinks/jsonrpc-cpp/actions/workflows/ci.yml)
[![CodeQL](https://github.com/sjinks/jsonrpc-cpp/actions/workflows/codeql.yml/badge.svg)](https://github.com/sjinks/jsonrpc-cpp/actions/workflows/codeql.yml)

JSON-RPC 2.0 library for C++

## Introduction

This library provides a robust implementation of the JSON-RPC 2.0 protocol for C++. It allows you to easily handle JSON-RPC requests, and manage responses.

## Features

- Full support for JSON-RPC 2.0
- Easy-to-use API
- Error handling and validation

## Installation

To install the library, follow these steps:

1. Clone the repository:
    ```sh
    git clone https://github.com/sjinks/jsonrpc-cpp.git
    cd jsonrpc-cpp
    ```

2. The library depends on [`nlohmann-json`](https://github.com/nlohmann/json). However, if you do not have it installed, it will be downloaded from the GitHub repository.

3. Build the project using CMake:
    ```sh
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ```

4. Install the library:
    ```sh
    cmake --install build # In Linux, you may have to use `sudo`
    ```

## Usage

### Basic Usage

```cpp
#include <wwa/jsonrpc/dispatcher.h>

class my_server {
public:
    my_server()
    {
        this->m_dispatcher.add("add", &my_server::add, this);
    }

    void handle_request()
    {
        // Read the request somehow
        const std::string input = read_request();

        try {
            const auto json     = nlohmann::json::parse(input);
            const auto result   = this->m_dispatcher.process_request(json);
            const auto response = wwa::json_rpc::serialize_repsonse(result);
            if (!response.empty()) {
                // Send the response
                send_response(response);
            }
        }
        catch (const nlohmann::json::exception& e) {
            send_response(
                wwa::json_rpc::generate_error_response(
                    {wwa::json_rpc::exception::PARSE_ERROR, e.what()}
                )
            );
        }
    }

private:
    wwa::json_rpc::dispatcher m_dispatcher;

    int add(int a, int b)
    {
        return a + b;
    }
};
```

### Advanced Usage

Sometimes, it may be necessary to pass some additional information to the handler. For example, an IP address of the client or authentication information.

Method handlers can accept a `context` parameter. That parameter is constructed from the data passed `dispatcher::process_request()` method and
additional fields from the JSON RPC request.

For example,

```cpp
struct extra_data {
    std::string ip;
};

class my_server {
public:
    my_server()
    {
        this->m_dispatcher.add_ex("add", &my_server::add, this);
    }

    void handle_request()
    {
        // Read the request somehow
        const nlohmann::json input = read_request();

        extra_data extra;
        extra.ip = get_peer_ip(); // Returns the IP of the client

        const auto response = wwa::json_rpc::serialize_repsonse(this->m_dispatcher.process_request(input, extra));
        if (!response.empty()) {
            // Send the response
            send_response(response);
        }
    }

private:
    wwa::json_rpc::dispatcher m_dispatcher;

    int add(const wwa::json_rpc::dispatcher::context_t& ctx, int a, int b)
    {
        std::cout << "IP address is " << std::any_cast<extra_data>(ctx.first).ip << "\n";
        return a + b;
    }
};
```

It is also possible to get the extra fields form the JSON RPC request.

For example, given the request:

```json
{
    "jsonrpc": "2.0",
    "method": "subtract",
    "params": {"minuend": 42, "subtrahend": 23},
    "id": 1,
    "auth": "secret",
    "user": "admin"
}
```

There are extra fields in the request: `auth` and `user`. These fields will be collected into an object and passed as a part of the context.

```cpp
int my_method(const wwa::json_rpc::dispatcher::context_t& ctx, int a, int b)
{
    const auto& extra = ctx.second;
    std::string auth = extra.at("auth");
    std::string user = extra.at("user");
    // ...
}
```

There are more examples available in the [test](https://github.com/sjinks/jsonrpc-cpp/tree/master/test) subdirectory
(you may want to look at `base.h`/`base.cpp` or `test_extra_param.cpp`)

The documentation is available at https://sjinks.github.io/jsonrpc-cpp/
