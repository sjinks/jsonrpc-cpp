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

        const std::string response = this->m_dispatcher.parse_and_process_request(input);
        if (!response.empty()) {
            // Send the response
            send_response(response);
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

Method handlers can accept an `extra` parameter of `nlohmann::json` type or a type convertible from `nlohmann::json`. That parameter is passed from
`dispatcher::parse_and_process_request()` and `dispatcher::process_request()` methods.

For example,

```cpp
struct extra_data {
    std::string ip;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(extra_data, ip);

class my_server {
public:
    my_server()
    {
        this->m_dispatcher.add_ex("add", &my_server::add, this);
    }

    void handle_request()
    {
        // Read the request somehow
        const std::string input = read_request();

        extra_data extra;
        extra.ip = get_peer_ip(); // Returns the IP of the client

        const std::string response = this->m_dispatcher.parse_and_process_request(input, extra);
        if (!response.empty()) {
            // Send the response
            send_response(response);
        }
    }

private:
    wwa::json_rpc::dispatcher m_dispatcher;

    int add(const extra_data& extra, int a, int b)
    {
        std::cout << "IP address is " << extra_data.ip << "\n";
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

There are extra fields in the request: `auth` and `user`.

If the `extra` parameter passed to `dispatcher::parse_and_process_request()` or `dispatcher::process_request()` is an object (in JSON terms),
the library will pass those extra fields in the `extra` property of the `extra` parameter.

For example, for the request above, the `extra` parameter will look like this:

```json
{
    // data passed to `dispatcher::parse_and_process_request()` or `dispatcher::process_request()`
    // ...

    "extra": {
        "auth": "secret",
        "user": "admin"
    }
}
```

The `extra_data` structure can be modified to include that field:

```cpp
struct extra_data {
    std::string ip;
    nlohmann::json extra;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(extra_data, ip, extra);
```

Or like this:

```cpp
struct extra_request_fields {
    std::string auth;
    std::string user;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(extra_request_fields, auth, user);

struct extra_data {
    std::string ip;
    extra_request_fields extra;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(extra_data, ip, extra);
```

There are more examples available in the [test](https://github.com/sjinks/jsonrpc-cpp/tree/master/test) subdirectory
(you may want to look at `base.h`/`base.cpp` or `test_extra_param.cpp`)

The documentation is available at https://sjinks.github.io/jsonrpc-cpp/
