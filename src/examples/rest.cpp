// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <asio.hpp> // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <iostream>
#include "server.hpp"

int main()
{
  try
  {
    asio::io_context ioc;

    f16::http::server::server http_server(ioc, "0.0.0.0", "7000", ".");
    http_server.add_handler("/version", [](std::ostream& os)
      {
          os << "1.0.0\n";
      });
    http_server.add_handler("/hello", [](std::ostream& os)
      {
          os << "Hello, world!\n";
      });

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
