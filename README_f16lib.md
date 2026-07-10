# f16 library (f16lib)

## About f16lib

`f16lib` is a lightweight C++17 library to build HTTP/HTTPS servers.
It provides routing primitives for dynamic handlers and static content serving.

## Key concepts

- `http_server` / `https_server`: server entry points.
- `path_router`: route matcher by method and path.
- `dynamic_content`: `GET`, `POST`, `PUT` handlers implemented as lambdas.
- `static_content`: filesystem-backed static resources.
- `request`: path parameters (`resource`) and query parameters (`query`).

## Public API Headers

All public API headers are located in `include/f16/`:

- **`http_server.hpp`** / **`https_server.hpp`**: Main entry points for creating HTTP and HTTPS servers.
- **`path_router.hpp`**: Route definition and matching.
- **`dynamic_content.hpp`**: User-defined handler types for dynamic responses.
- **`static_content.hpp`**: Static file serving configuration.
- **`request.hpp`** / **`http_request.hpp`**: Request type with path/query parameters.
- **`reply.hpp`**: Response type.
- **`logger.hpp`**: Logging configuration.
- **`header.hpp`** / **`f16asio.hpp`** / **`mime_types.hpp`**: Supporting types and utilities.

## Example applications

The repository includes ready-to-run examples in `src/examples`:

- `static.cpp`: static content server.
- `https_static.cpp`: HTTPS static server.
- `rest.cpp`: dynamic REST-style routes.
- `complete.cpp`: mixed static + dynamic server.
- `blackbox_app.cpp`: complete sample used by black-box tests.

Each example is built as an executable named `example-<name>` (for example `example-rest`).

## Build and run an example

```shell
cmake -S . -B ./build
cmake --build ./build --target example-rest
./build/src/examples/example-rest
```

## Black-box validation sample

`src/examples/blackbox_app.cpp` is a full sample that combines:

- dynamic routes,
- static content under `/static`,
- query and path parameter parsing,
- custom statuses/content types,
- header inspection.

This sample is validated end-to-end by Python tests in `tests/blackbox/test_blackbox.py`.

Run only the black-box suite:

```shell
cd ./build
ctest -R blackbox --output-on-failure
cd ../
```

## Using f16lib as an installed library (CMake package)

### Installation

After building the project, install f16 to a prefix:

```bash
cmake --install ./build --prefix /path/to/install
```

This installs:
- Library: `/path/to/install/lib/libf16lib.a`
- Public headers: `/path/to/install/include/f16/*.hpp`
- CMake package files: `/path/to/install/lib/cmake/f16/`

### Using in Your Project

In your application's `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_app LANGUAGES CXX)

find_package(f16 CONFIG REQUIRED)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE f16::f16lib)
```

Configure and build with correct dependency paths:

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="/path/to/install;/path/to/asio/install"

cmake --build build
```

Include public headers in your code:

```cpp
#include <f16/http_server.hpp>
#include <f16/path_router.hpp>
#include <f16/dynamic_content.hpp>

// Your application code...
int main() {
    f16::http::server::http_server server;
    // ...
}
```

### Notes

- The `f16` package config resolves transitive dependencies using `find_dependency()`.
- Ensure your environment can find both `asio` and `OpenSSL` (if using HTTPS).
- The library uses C++17 and requires a compatible compiler.

## Hardening defaults

Starting from the 1.0 hardening work, `http_server` and `https_server` include built-in request guardrails via `server_options`.

Default values:

- `max_request_line_bytes`: `8192`
- `max_header_section_bytes`: `32768`
- `max_headers_count`: `100`
- `max_body_bytes`: `1048576` (1 MiB)
- `read_header_timeout`: `15s`
- `read_body_timeout`: `30s`
- `tls_handshake_timeout`: `10s`

Behavior:

- Invalid request syntax returns `400 Bad Request`.
- Request limits exceeded returns `413 Request Entity Too Large`.
- Read timeout expiry returns `408 Request Timeout`.

## Tuning example

For internet-facing deployments, tighten limits and timeouts based on your API profile:

```cpp
#include <chrono>
#include <f16/http_server.hpp>

asio::io_context ioc;

f16::http::server::server_options options;
options.max_request_line_bytes = 4096;
options.max_header_section_bytes = 16384;
options.max_headers_count = 64;
options.max_body_bytes = 256 * 1024;
options.read_header_timeout = std::chrono::seconds(5);
options.read_body_timeout = std::chrono::seconds(10);

f16::http::server::http_server server(ioc, options);
```

For trusted internal traffic, you can relax these values where needed.

## Related docs

- Main project guide: [README.md](README.md)
- Server CLI guide: [README_f16server.md](README_f16server.md)
