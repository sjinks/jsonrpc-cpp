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

2. Build the project using CMake:
    ```sh
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ```

3. Install the library:
    ```sh
    cmake --install build # In Linux, you may have to use `sudo`
    ```

## Usage

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

        const std::string response = this->m_dispatcher->parse_and_process_request(input);
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
