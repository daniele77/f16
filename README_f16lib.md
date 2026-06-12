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
cmake -S . -B ./build_conan
cmake --build ./build_conan --target example-rest
./build_conan/src/examples/example-rest
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
cd ./build_conan
ctest -R blackbox --output-on-failure
cd ../
```

## Using f16lib as an installed library (CMake package)

### Installation

After building the project, install f16 to a prefix:

```bash
cmake --install ./build_conan --prefix /path/to/install
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

## Related docs

- Main project guide: [README.md](README.md)
- Server CLI guide: [README_f16server.md](README_f16server.md)
