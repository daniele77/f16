// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <asio.hpp> // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <iostream>
#include "server.hpp"
#include "dynamic_content_get.hpp"

int main()
{
  try
  {
    asio::io_context ioc;

    using namespace f16::http::server;
    server http_server(ioc);
    http_server.add("GET", "/version", std::make_shared<dynamic_content_get>([](std::ostream& os) { os << "1.0.0\n"; }));
    http_server.add("GET", "/hello", std::make_shared<dynamic_content_get>([](std::ostream& os) { os << "Hello, world!\n"; }));
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
