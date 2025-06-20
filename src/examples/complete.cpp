// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <iostream>
#include "http_server.hpp"
#include "static_content.hpp"
#include "dynamic_content.hpp"

int main(int /*argc*/, const char** /*argv*/)
{
  try
  {
    asio::io_context ioc;

    using namespace f16::http::server;
    http_server app(ioc);
    path_router router;
    // Serve static content from the current directory
    router.add("/", static_content("."));
    // Add a dynamic content handler for the "/books" endpoint
    router.add("/books", get([](const request& /*req*/, std::ostream& os) {
      os << "Hello, get api!\n";
    }));
    // Set the router to the server
    app.set(std::move(router));
    // Start listening on port 7000
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
    return 1;
  }
  return 0;
}
