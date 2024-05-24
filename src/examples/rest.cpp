// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma GCC diagnostic ignored "-Wnull-dereference"
#include <asio.hpp> // NB: the asio header must be included *before* iostream to avoid sanity check error
#pragma GCC diagnostic warning "-Wnull-dereference"
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

    http_server.add("/version", get([](const request& /*req*/, std::ostream& os) { os << "1.0.0\n"; }));
    http_server.add("/hello", get([](const request& /*req*/, std::ostream& os) { os << "Hello, world!\n"; }));
    http_server.add("/greet", get({"name", "country"}, [](const request& req, std::ostream& os) {
        try
        {
          os << "Hi, " << req.param("name") << " from " << req.param("country") << "!\n";
        }
        catch(const std::exception& e)
        {
          os << "Error in request parameters\n";
        }
      })
    );
    http_server.add("/person", put({"name", "country"}, [](const request& req, std::ostream& os) {
        try
        {
          std::cout << "Insert person " << req.param("name") << " from " << req.param("country") << "\n";
          os << "ok";
        }
        catch(const std::exception& e)
        {
          os << "Error in request parameters\n";
        }
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
