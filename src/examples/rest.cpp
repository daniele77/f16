// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <iostream>
#include "http_server.hpp"
#include "dynamic_content.hpp" // get, post, put
#include "request.hpp"

int main()
{
  try
  {
    asio::io_context ioc;

    using namespace f16::http::server;
    http_server server(ioc);

    path_router router;
    // GET <ip>/version
    router.add("/version", get([](const request& /*req*/, std::ostream& os) { os << "1.0.0\n"; }));
    router.add("/version_j", get([](const request& /*req*/, f16::response_stream& os) { os << f16::json << R"({"version":"1.0.0"})"; }));
    router.add("/bad", get([](const request& /*req*/, f16::response_stream& os) { os << f16::bad_request; }));
    // GET <ip>/hello
    router.add("/hello", get([](const request& /*req*/, std::ostream& os) { os << "Hello, world!\n"; }));
    router.add("/hello", get({"name"}, [](const request& req, std::ostream& os) { os << "Hello, " << req.resource("name") << "!\n"; }));
    // GET <ip>/print/?name=<name>&country=<country>
    router.add("/print", get([](const request& req, std::ostream& os) {
        os << "Hi, " << req.query("name") << " from " << req.query("country") << "!\n";
      })
    );
    // GET <ip>/greet/<name>/<country>
    router.add("/greet", get({"name", "country"}, [](const request& req, std::ostream& os) {
        os << "Hi, " << req.resource("name") << " from " << req.resource("country") << "!\n";
      })
    );
    // PUT <ip>/person/<name>/<country>
    router.add("/person", put({"name", "country"}, [](const request& req, std::ostream& os) {
        std::cout << "Insert person " << req.resource("name") << " from " << req.resource("country") << "\n";
        os << "ok";
      })
    );

    server.set(std::move(router));

    // Start listening on port 7000
    server.listen("7000", "0.0.0.0");

    while(true)
    {
        try
        {
            ioc.run();
            break; // run() exited normally
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception caugth in io_context scheduler: " << e.what() << std::endl;
        }
    }        

  }
  catch (const std::exception &e)
  {
    std::cerr << "Unhandled exception in main: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
