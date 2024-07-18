// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <iostream>
#include "server.hpp"
#include "dynamic_content.hpp"

int main()
{
  try
  {
    asio::io_context ioc;

    using namespace f16::http::server;
    server http_server(ioc);

    // GET <ip>/version
    http_server.add("/version", get([](const request& /*req*/, std::ostream& os) { os << "1.0.0\n"; }));
    // GET <ip>/hello
    http_server.add("/hello", get([](const request& /*req*/, std::ostream& os) { os << "Hello, world!\n"; }));
    // GET <ip>/print/?name=<name>&country=<country>
    http_server.add("/print", get([](const request& req, std::ostream& os) {
        os << "Hi, " << req.query("name") << " from " << req.query("country") << "!\n";
      })
    );
    // GET <ip>/greet/<name>/<country>
    http_server.add("/greet", get({"name", "country"}, [](const request& req, std::ostream& os) {
        os << "Hi, " << req.resource("name") << " from " << req.resource("country") << "!\n";
      })
    );
    // PUT <ip>/person/<name>/<country>
    http_server.add("/person", put({"name", "country"}, [](const request& req, std::ostream& os) {
        std::cout << "Insert person " << req.resource("name") << " from " << req.resource("country") << "\n";
        os << "ok";
      })
    );

    http_server.listen("7000", "0.0.0.0");

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
  }

  return 0;
}
