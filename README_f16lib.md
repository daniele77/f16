# f16 library (f16lib)

## About f16lib

f16lib is a versatile and lightweight C++17 library for building efficient HTTP and HTTPS servers, ideal for REST APIs and custom web applications.
It provides a simple and intuitive API for defining routes, handling requests, and serving static or dynamic content.

### Basic Server:

This example demonstrates a basic HTTP server that listens on port 7000 and serves the content of the current directory:

```c++
#include "f16asio.hpp"
#include <iostream>
#include "static_content.hpp"

int main() {
  try {
    asio::io_context ioc;

    using namespace f16::http::server;
    http_server app(ioc);
    app.add("/", static_content("."));
    app.listen("7000", "localhost");

    ioc.run();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```

### Handling Requests:

You can define routes that handle specific HTTP methods and extract data from requests. Here's an example that serves different content based on the requested path:

```c++
#include "f16asio.hpp"
#include <iostream>
#include "http_server.hpp"
#include "dynamic_content.hpp"

int main() {
  try {
    asio::io_context ioc;

    using namespace f16::http::server;
    http_server server(ioc);

    // GET /version
    server.add("/version", get([](const request& /*req*/, std::ostream& os) {
      os << "1.0.0\n";
    }));

    // GET /hello
    server.add("/hello", get([](const request& /*req*/, std::ostream& os) {
      os << "Hello, world!\n";
    }));

    server.listen("7000", "0.0.0.0");

    ioc.run();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```

In this example, the `/version` route returns the server version, and the `/hello` route returns a simple greeting message.

### Extracting Query Parameters and Path Variables:

f16 allows you to extract query parameters and path variables from requests. Here's an example that greets a user by name and country:

```c++
#include "f16asio.hpp"
#include <iostream>
#include "http_server.hpp"

int main() {
  try {
    asio::io_context ioc;

    using namespace f16::http::server;
    http_server server(ioc);

    // GET /print/?name=<name>&country=<country>
    server.add("/print", get([](const request& req, std::ostream& os) {
      os << "Hi, " << req.query("name") << " from " << req.query("country") << "!\n";
    }));

    server.listen("7000", "0.0.0.0");

    ioc.run();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```

Similarly, you can define routes that capture path variables:

```c++
#include "f16asio.hpp"
#include <iostream>
#include "http_server.hpp"

int main() {
  try {
    asio::io_context ioc;

    using namespace f16::http::server;
    http_server server(ioc);

    // GET /greet/<name>/<country>
    server.add("/greet", get({"name", "country"}, [](const request& req, std::ostream& os) {
        os << "Hi, " << req.resource("name") << " from " << req.resource("country") << "!\n";
      })
    );    
```
