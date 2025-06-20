// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <functional>
#include <fstream>
#include <iostream>
#include <csignal>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include "http_server.hpp"
#include "https_server.hpp"

#include "static_content.hpp"

// This file will be generated automatically when you run the CMake configuration step.
// It creates a namespace called `f16`.
// You can modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>


static f16::http::server::ssl_settings::ssl_proto protocol_from_string(const std::string& s)
{
  using namespace f16::http::server;
  if (s == "SSLv2") return ssl_settings::sslv2;
  if (s == "SSLv3") return ssl_settings::sslv3;
  if (s == "TLSv1") return ssl_settings::tlsv1;
  if (s == "TLSv1.1") return ssl_settings::tlsv11;
  if (s == "TLSv1.2") return ssl_settings::tlsv12;
  if (s == "TLSv1.3") return ssl_settings::tlsv13;
  throw std::invalid_argument("Unknown protocol: " + s);
}

static constexpr auto USAGE =
  R"(f16 web server.

    Usage:
          f16 simple <root_doc> [--bind=<address> --port=<port>]
          f16 advanced <cfg_file>
          f16 (-h | --help)
          f16 --version

    Options:
          -h --help            Show this screen.
          -v --version         Show version.
          -b --bind=<address>  The binding address [default: 0.0.0.0].
          -p --port=<port>     The port [default: 80].
  )";

int main(int argc, const char **argv)
{
  try {
    std::map<std::string, docopt::value> args = 
      docopt::docopt(
        USAGE,
        { std::next(argv), std::next(argv, argc) },
        true,// show help if requested
        fmt::format("{} {}",
          f16::cmake::project_name,
          f16::cmake::project_version));// version string, acquired from config.hpp via CMake

    // for (auto const &arg : args) { std::cout << arg.first << "=" << arg.second << '\n'; }

    // Use the default logger (stdout, multi-threaded, colored)
    // spdlog::info("Hello, {}!", "World");

    // fmt::print("Hello, from {}\n", "{fmt}");

    // http server
    asio::io_context ioc;
    
    using namespace f16::http::server;

    std::vector<std::unique_ptr<http_server>> server_set;

    if (args["simple"].asBool())
    {
      const std::string address = args["--bind"].asString();
      const std::string port = args["--port"].asString();

      const std::string root_doc = args["<root_doc>"].asString();
      spdlog::info("Serving root doc {} on {}:{}", root_doc, address, port);

      auto server = std::make_unique<http_server>(ioc);

      path_router router;
      router.add("/", static_content(root_doc));
      server->set(std::move(router));
      server->listen(port, address);

      server_set.push_back(std::move(server));
    }
    else if (args["advanced"].asBool())
    {
      const std::string cfg_file = args["<cfg_file>"].asString();
      std::ifstream ifs(cfg_file);
      if (!ifs) throw std::runtime_error{"Configuration file " + cfg_file + " not fouund"};
      const nlohmann::json cfg = nlohmann::json::parse(ifs,
        nullptr, // callback
        true, // allow exceptions
        true // ignore_comments
      );

      for (const auto& server_entry : cfg.at("servers"))
      {
        const std::string address = server_entry.at("listen_address");
        const bool has_ssl = server_entry.contains("ssl");
        const std::string port = server_entry.value("listen_port", (has_ssl ? "443" : "80"));
        spdlog::info("New {} server listening on {}:{}", (has_ssl ? "https" : "http"), address, port);
        std::unique_ptr<http_server> server;
        if (has_ssl)
        {
          const auto& ssl_section = server_entry.at("ssl");
          ssl_settings ssl_s;
          ssl_s.certificate = ssl_section.value("certificate", "");
          ssl_s.certificate_key = ssl_section.value("certificate_key", "");
          ssl_s.dhparam = ssl_section.value("dhparam", "");
          ssl_s.password = ssl_section.value("password", "");
          if (ssl_section.contains("protocols"))
          {
            const auto& protocols = ssl_section["protocols"];
            for (std::string protocol: protocols)
              ssl_s.protocols.insert(protocol_from_string(protocol));
          }

          ssl_s.ciphers = ssl_section.value("ciphers", "");
          ssl_s.prefer_server_ciphers = ssl_section.value("prefer_server_ciphers", false);

          if (ssl_section.value("verify_client", false))
            ssl_s.client_certificate = ssl_section.value("client_certificate", "");

          ssl_s.session_timeout_secs = ssl_section.value("session_timeout_secs", -1L);
          ssl_s.session_cache = ssl_section.value("session_cache", false);
          ssl_s.session_cache_size = ssl_section.value("session_cache_size", -1L);

          server = std::make_unique<https_server>(ioc, ssl_s);
        }
        else server = std::make_unique<http_server>(ioc);

        path_router router;
        for (const auto& location_entry: server_entry.at("locations"))
        {
          const std::string root_doc = location_entry.at("root");
          const std::string path = location_entry.at("location");
          spdlog::info("  Serving root doc {} at path: {}", root_doc, path);
          router.add(path, static_content(root_doc));
        }
        server->set(std::move(router));
        server->listen(port, address);
        server_set.push_back(std::move(server));
      }
    }
      
    // Register to handle the signals that indicate when the app should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    asio::signal_set signals_(ioc);
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait([&ioc](std::error_code /*ec*/, int /*signo*/){ ioc.stop(); });

    //  start app

    spdlog::info("Start application");

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

    spdlog::info("Gracefully exit application");

  } catch (const std::exception &e) {
    fmt::print(stderr, "Unhandled exception in main: {}", e.what());
    exit(1);
  }

  return 0;
}
