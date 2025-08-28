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

#include "http_request.hpp"
#include "mime_types.hpp"

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
      if (!ifs)
        throw std::runtime_error{"Configuration file " + cfg_file + " not fouund"};
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
        else
          server = std::make_unique<http_server>(ioc);

        if (server_entry.contains("return"))
        {
          const auto& return_section = server_entry.at("return");
          const std::string status_s = return_section.value("status", "ok");
          const reply::status_type status = 
          (status_s == "ok" ? reply::ok :
            status_s == "created" ? reply::created :
            status_s == "accepted" ? reply::accepted :
            status_s == "no_content" ? reply::no_content :
            status_s == "multiple_choices" ? reply::multiple_choices :
            status_s == "moved_permanently" ? reply::moved_permanently :
            // status_s == "found" ? reply::found :
            // status_s == "see_other" ? reply::see_other :
            status_s == "not_modified" ? reply::not_modified :
            status_s == "bad_request" ? reply::bad_request :
            status_s == "unauthorized" ? reply::unauthorized :
            status_s == "forbidden" ? reply::forbidden :
            status_s == "not_found" ? reply::not_found :
            status_s == "internal_server_error" ? reply::internal_server_error :
            status_s == "not_implemented" ? reply::not_implemented :
            status_s == "bad_gateway" ? reply::bad_gateway :
            status_s == "service_unavailable" ? reply::service_unavailable :
            /*else*/ reply::ok);
            spdlog::info("  Serving status '{}'", status_s);
            const auto headers = return_section.value("headers", nlohmann::json::array());
            server->set(
              [status, headers](const http_request& req, reply& res)
              {
                res = reply::stock_reply(status);
                for (const auto& header_entry: headers)
                {
                  std::string name;
                  std::string value;
                  for (auto& [k,v] : header_entry.items()) {
                    name = k;
                    value = v;
                  }
                  
                  // replace $host and $request_uri
                  if (value.find("$host") != std::string::npos)
                  {
                    std::string host = req.get_header("host");
                    if (host.empty())
                    {
                      res = reply::stock_reply(reply::bad_request); // 400
                      res.content = "Missing 'Host' header in the request";
                      res.headers = {
                        {"Content-Length", std::to_string(res.content.size())},
                        {"Content-Type", mime_types::extension_to_type(".txt")}
                      };
                      return;
                    }
                    if (auto pos = host.find(':'); pos != std::string::npos)
                    host.erase(pos); // Erases everything after the ':' character
                    value.replace(value.find("$host"), 5, host);
                  }
                  if (value.find("$request_uri") != std::string::npos)
                  {
                    value.replace(value.find("$request_uri"), 12, req.uri);
                  }
                  res.headers.push_back({name, value});
                }
                
                /*
                spdlog::info("  Request: {} {}", req.method, req.uri);
                for (const auto& h: req.headers)
                spdlog::info("    {}: {}", h.name, h.value);
                spdlog::info("  Response: {} {}", static_cast<int>(res.status), res.content);
                for (const auto& h: res.headers)
                spdlog::info("    {}: {}", h.name, h.value);
                */
              }
            );
        }
        else if (server_entry.contains("locations"))
        {
          path_router router;
          for (const auto& location_entry: server_entry.at("locations"))
          {
            const std::string root_doc = location_entry.at("root");
            const std::string path = location_entry.at("location");
            spdlog::info("  Serving root doc {} at path: {}", root_doc, path);
            router.add(path, static_content(root_doc));
          }
          server->set(std::move(router));
        }
        else
        {
          spdlog::warn("No 'return' or 'locations' section found for this server entry: this server will not handle any request"); 
        }
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
