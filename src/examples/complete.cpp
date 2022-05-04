// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <asio.hpp> // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <iostream>
#include "server.hpp"

int main(int /*argc*/, const char** /*argv*/)
{
  try
  {
    asio::io_context ioc;

#if 0
    f16::http::server::server app(ioc);
    app.static("/", "/var/www/");
    app.get("/books", [](req, res){
      res.json(books);
    });
    app.post("/book", [](req, res){
    // We will be coding here
    });
#else
    f16::http::server::server app(ioc, ".");
    app.listen("7000", "localhost");
#endif

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
