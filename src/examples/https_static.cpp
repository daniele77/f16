// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <iostream>
#include "https_server.hpp"
#include "http_server.hpp"
#include "static_content.hpp"
#include "reply.hpp"
#include "http_request.hpp"

int main(int /*argc*/, const char** /*argv*/)
{
  try
  {
    asio::io_context ioc;

    using namespace f16::http::server;

    // openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365
    // psw -> 123456
    //
    // openssl dhparam -out dh4096.pem 4096

    const ssl_settings ssl_s{
      "cert.pem",
      "key.pem",
      "dh4096.pem"
    };
    https_server server{ioc, ssl_s};
    path_router router;
    router.add("/", static_content("."));
    server.set(std::move(router));
    server.listen("7000", "0.0.0.0");

    // http server redirects to https
    http_server http_server{ioc};
    http_server.set(
      [](const http_request& req, reply& res)
      {
        std::string host = req.get_header("host");
        if (host.empty()) // no host header
        {
          res = reply::stock_reply(reply::bad_request); // 400
        }
        else
        {
          if (auto pos = host.find(':'); pos != std::string::npos)
            host.erase(pos); // Erases everything after the ':' character
          res = reply::stock_reply(reply::moved_permanently); // 301
          res.headers.push_back({"Location", "https://" + host + ":7000" + req.uri});
        }
      }
    );
    http_server.listen("8002", "0.0.0.0");

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
