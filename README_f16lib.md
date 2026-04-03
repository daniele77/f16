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

## Related docs

- Main project guide: [README.md](README.md)
- Server CLI guide: [README_f16server.md](README_f16server.md)
