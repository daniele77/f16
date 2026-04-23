// Copyright (c) 2026 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "dynamic_content.hpp"
#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include "http_server.hpp"
#include "request.hpp"
#include "static_content.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{

struct config
{
  std::string host = "127.0.0.1";
  std::string port = "18080";
  std::string doc_root = ".";
};

config parse_args(int argc, char* argv[])
{
  config c;

  for (int i = 1; i < argc; ++i)
  {
    const std::string arg = argv[i];
    if (arg == "--host" && i + 1 < argc)
    {
      c.host = argv[++i];
      continue;
    }
    if (arg == "--port" && i + 1 < argc)
    {
      c.port = argv[++i];
      continue;
    }
    if (arg == "--doc-root" && i + 1 < argc)
    {
      c.doc_root = argv[++i];
      continue;
    }

    std::cerr << "Unknown/invalid argument: " << arg << '\n'
              << "Usage: " << argv[0] << " [--host <ip>] [--port <port>] [--doc-root <path>]\n";
    std::exit(2);
  }

  return c;
}

} // namespace

int main(int argc, char* argv[])
{
  try
  {
    const config cfg = parse_args(argc, argv);

    asio::io_context ioc;

    using namespace f16::http::server;
    http_server app(ioc);

    path_router router;

    router.add("/health", get([](const request&, f16::response_stream& os) {
      os << f16::json << R"({"status":"ok"})";
    }));

    router.add("/hello/:name", get([](const request& req, f16::response_stream& os) {
      os << "Hello " << req.resource("name") << "!";
    }));

    router.add("/sum", get([](const request& req, f16::response_stream& os) {
      const auto a_str = req.query("a");
      const auto b_str = req.query("b");
      if (a_str.empty() || b_str.empty())
      {
        os << f16::bad_request << "missing query parameters";
        return;
      }

      try
      {
        const int a = std::stoi(a_str);
        const int b = std::stoi(b_str);
        os << f16::json << "{\"sum\":" << (a + b) << "}";
      }
      catch (const std::exception&)
      {
        os << f16::bad_request << "invalid integers";
      }
    }));

    router.add("/status/bad", get([](const request&, f16::response_stream& os) {
      os << f16::bad_request;
    }));

    router.add("/items/:id", post([](const request& req, f16::response_stream& os) {
      os << f16::json << R"({"method":"POST","id":")" << req.resource("id") << R"("})";
    }));

    router.add("/items/:id", put([](const request& req, f16::response_stream& os) {
      os << f16::json << R"({"method":"PUT","id":")" << req.resource("id") << R"("})";
    }));

    router.add("/path/:var1/to/:var2", get([](const request& req, f16::response_stream& os) {
      os << f16::json << R"({"method":"GET","parameters":"yes","var1":")" << req.resource("var1") << R"(","var2":")" << req.resource("var2") << R"("})";
    }));

    router.add("/path", get([](const request& /*req*/, f16::response_stream& os) {
      os << f16::json << R"({"method":"GET","parameters":"no"})";
    }));

    router.add("/echo", post([](const request& req, f16::response_stream& os) {
      os << f16::plain << req.orig_request.body;
    }));

    router.add("/headers/user-agent", get([](const request& req, f16::response_stream& os) {
      os << req.orig_request.get_header("user-agent");
    }));

    router.add("/static", static_content(cfg.doc_root));

    app.set(std::move(router));
    app.listen(cfg.port, cfg.host);

    while (true)
    {
      try
      {
        ioc.run();
        break;
      }
      catch (const std::exception& e)
      {
        std::cerr << "Exception caught in io_context scheduler: " << e.what() << '\n';
      }
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Unhandled exception in main: " << e.what() << '\n';
    return 1;
  }

  return 0;
}