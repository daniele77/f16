// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>

#include <CLI/CLI.hpp>
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


using namespace f16::http::server;

static f16::http::server::ssl_settings::ssl_proto protocol_from_string(const std::string& s)
{
  if (s == "SSLv2") return ssl_settings::sslv2;
  if (s == "SSLv3") return ssl_settings::sslv3;
  if (s == "TLSv1") return ssl_settings::tlsv1;
  if (s == "TLSv1.1") return ssl_settings::tlsv11;
  if (s == "TLSv1.2") return ssl_settings::tlsv12;
  if (s == "TLSv1.3") return ssl_settings::tlsv13;
  throw std::invalid_argument("Unknown protocol: " + s);
}

static void build_simple_server(asio::io_context& ioc, std::vector<std::unique_ptr<http_server>>& server_set, const std::string& root_doc, const std::string& bind_address, int port)
{
  spdlog::info("Serving root doc {} on {}:{}", root_doc, bind_address, port);

  auto server = std::make_unique<http_server>(ioc);

  path_router router;
  router.add("/", static_content(root_doc));
  server->set(std::move(router));
  server->listen(std::to_string(port), bind_address);

  server_set.push_back(std::move(server));
}

static void build_advanced_server(asio::io_context& ioc, std::vector<std::unique_ptr<http_server>>& server_set, const std::string& cfg_file)
{
  std::ifstream ifs(cfg_file);
  if (!ifs)
    throw std::runtime_error{ "Configuration file " + cfg_file + " not found" };
  const nlohmann::json jcfg = nlohmann::json::parse(ifs,
    nullptr, // callback
    true, // allow exceptions
    true // ignore_comments
  );

  for (const auto& server_entry : jcfg.at("servers"))
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
        for (std::string protocol : protocols)
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
      const reply::status_type status = reply::status_from_string(status_s);
      spdlog::info("  Serving status '{}'", status_s);
      const auto headers = return_section.value("headers", nlohmann::json::array());
      server->set(
        [status, headers](const http_request& req, reply& res) {
          res = reply::stock_reply(status);
          for (const auto& header_entry : headers)
          {
            std::string name;
            std::string value;
            for (auto& [k, v] : header_entry.items())
            {
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
                  { "Content-Length", std::to_string(res.content.size()) },
                  { "Content-Type", mime_types::extension_to_type(".txt") }
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
            res.headers.push_back({ name, value });
          }
        });
    }
    else if (server_entry.contains("locations"))
    {
      path_router router;
      for (const auto& location_entry : server_entry.at("locations"))
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

int main(int argc, const char** argv)
{
  CLI::App app{ "f16 web server" };
  try
  {
    app.require_subcommand(0); // Allows 0 or 1 subcommand

    app.add_flag(
      "-v,--version",
      [](auto /* count */) {
        fmt::print("{} web server v. {}\n",
          f16::cmake::project_name,
          f16::cmake::project_version); // version string, acquired from config.hpp via CMake
        std::exit(0);
      },
      "Show version.");

    // --- Subcommand 1: SIMPLE ---
    CLI::App* serve_cmd = app.add_subcommand("serve", "Simple http server mode, serving a directory.");

    std::string root_doc;
    int port = 80;
    std::string bind_address = "0.0.0.0";

    // Positional argument mandatory: <root_doc>
    serve_cmd->add_option("root_doc", root_doc, "Path of the folder to serve.")
      ->required()
      ->check(CLI::ExistingDirectory);

    // Optional flags
    serve_cmd->add_option("-p,--port", port, "The port [default: 80].")
      ->check(CLI::PositiveNumber); // the port must be positive
    serve_cmd->add_option("-b,--bind", bind_address, "The binding address [default: 0.0.0.0]");

    // --- Subcommand 2: ADVANCED ---
    CLI::App* config_cmd = app.add_subcommand("config", "Start the server using a configuration file.");
    std::string config_path;

    // Positional argument mandatory: <cfg_file>
    config_cmd->add_option("config_path", config_path, "Configurazione file path.")
      ->required()
      ->check(CLI::ExistingFile);

    app.parse(argc, argv);

    // http server
    asio::io_context ioc;

    std::vector<std::unique_ptr<http_server>> server_set;

    if (serve_cmd->parsed())
    {
      build_simple_server(ioc, server_set, root_doc, bind_address, port);
    }
    else if (config_cmd->parsed())
    {
      build_advanced_server(ioc, server_set, config_path);
    }
    else
    {
      fmt::print(app.help());
      return 1;
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
    signals_.async_wait([&ioc](std::error_code /*ec*/, int /*signo*/) { ioc.stop(); });

    //  start app

    spdlog::info("Start application");

    while (true)
    {
      try
      {
        ioc.run();
        break; // run() exited normally
      }
      catch (const std::exception& e)
      {
        fmt::print(stderr, "Exception caugth in io_context scheduler: {}", e.what());
      }
    }

    spdlog::info("Gracefully exit application");
  }
  catch (const CLI::ParseError& e)
  {
    // Print error message and cli help 
    return app.exit(e);
  }
  catch (const std::exception& e)
  {
    fmt::print(stderr, "Unhandled exception in main: {}", e.what());
    return 1;
  }

  return 0;
}
