// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma GCC diagnostic ignored "-Wnull-dereference"
#include <asio.hpp> // NB: the asio header must be included *before* iostream to avoid sanity check error
#pragma GCC diagnostic warning "-Wnull-dereference"
#include <iostream>
#include "server.hpp"
#include "static_content.hpp"
#include "dynamic_content.hpp"

int main(int /*argc*/, const char** /*argv*/)
{
  try
  {
    asio::io_context ioc;

    using namespace f16::http::server;
    server app(ioc);
    app.add("/", static_content("."));
    app.add("/books", get([](const request& /*req*/, std::ostream& os) {
      os << "Hello, get api!\n";
    }));
    app.listen("7000", "localhost");

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
}
