// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <csignal>
#include <exception>
#include <fstream>
#include <string>
// #include <vector>
// #include <utility> // for std::move
#include <chrono> // for std::chrono::milliseconds

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <nlohmann/json.hpp>

#include "http_server.hpp"
#include "https_server.hpp"
#include "nlohmann/json_fwd.hpp"
#include "spdlog_logger.hpp"

#include "static_content.hpp"

#include "http_request.hpp"
#include "mime_types.hpp"

#include "server_collection.hpp"
#include "logger_collection.hpp"

// This file will be generated automatically when you run the CMake configuration step.
// It creates a namespace called `f16`.
// You can modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>


using namespace f16::http::server;

static std::chrono::milliseconds flush_interval_from_config(const nlohmann::json& root_config)
{
  if (!root_config.contains("logs") || !root_config.at("logs").is_object())
  return std::chrono::milliseconds(1000);
  
  const auto& logs = root_config.at("logs");
  if (!logs.contains("flush_interval_ms"))
  return std::chrono::milliseconds(1000);
  
  const auto value = logs.value("flush_interval_ms", 1000);
  if (value <= 0)
    return std::chrono::milliseconds(1000);
  
  return std::chrono::milliseconds(value);
}

#if 0
static spdlog::level::level_enum flush_level_from_config(
  const nlohmann::json& logs,
  const char* section_name,
  spdlog::level::level_enum default_level)
  {
    if (!logs.contains(section_name) || !logs.at(section_name).is_object())
    return default_level;
    
    const auto& section = logs.at(section_name);
    if (!section.contains("flush_on_level"))
    return default_level;
    
    const std::string level_s = section.value("flush_on_level", "");
    if (level_s.empty())
    return default_level;
    
    return spdlog::level::from_str(level_s);
  }
  
  static std::vector<spdlog::sink_ptr> create_sinks_from_config(const nlohmann::json& section)
  {
    std::vector<spdlog::sink_ptr> sinks;
    
    if (!section.contains("sinks") || !section.at("sinks").is_array())
    return sinks;
    
    for (const auto& sink_cfg : section.at("sinks"))
    {
      const std::string type = sink_cfg.value("type", "");
      try
      {
        std::shared_ptr<spdlog::sinks::sink> sink;
        
        if (type == "stdout")
        {
          const bool color = sink_cfg.value("color", true);
          if (color)
          sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
          else
          sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        }
        else if (type == "stderr")
        {
          const bool color = sink_cfg.value("color", true);
          if (color)
          sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
          else
          sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
        }
        else if (type == "file")
        {
          const std::string path = sink_cfg.value("path", "");
          if (path.empty())
          continue;
          const std::size_t max_size = sink_cfg.value("max_size", static_cast<std::size_t>(10485760));
          const std::size_t max_files = sink_cfg.value("max_files", static_cast<std::size_t>(5));
          sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, max_size, max_files);
        }
        else if (type == "syslog")
        {
          spdlog::warn("syslog sink currently not supported in f16 server logger config; skipping");
          continue;
        }
        else
        {
          spdlog::warn("Unknown log sink type '{}'; skipping", type);
          continue;
        }
        
        if (sink_cfg.contains("level"))
        {
          const auto sink_level = spdlog::level::from_str(sink_cfg.value("level", "info"));
          sink->set_level(sink_level);
        }
        
        sinks.push_back(sink);
      }
      catch (const std::exception& e)
      {
        spdlog::warn("Failed to create sink of type '{}': {}", type, e.what());
      }
    }
    
    return sinks;
  }
  
  static logger_ptr create_logger_from_config(const std::string& server_name, const nlohmann::json& root_config)
  {
    nlohmann::json logs = nlohmann::json::object();
    if (root_config.contains("logs") && root_config.at("logs").is_object())
    {
      logs = root_config.at("logs");
    }
    
    std::vector<spdlog::sink_ptr> access_sinks;
    std::vector<spdlog::sink_ptr> error_sinks;
    
    if (logs.contains("access") && logs.at("access").is_object())
    {
      const auto& access_cfg = logs.at("access");
      const bool enabled = access_cfg.value("enabled", true);
      if (enabled)
      access_sinks = create_sinks_from_config(access_cfg);
    }
    
    if (logs.contains("error") && logs.at("error").is_object())
    {
      const auto& error_cfg = logs.at("error");
      const bool enabled = error_cfg.value("enabled", true);
      if (enabled)
      error_sinks = create_sinks_from_config(error_cfg);
    }
    
    if (access_sinks.empty())
    access_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    if (error_sinks.empty())
    error_sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
    
    auto access_backend = std::make_shared<spdlog::logger>(server_name + "_access", access_sinks.begin(), access_sinks.end());
    access_backend->set_level(spdlog::level::info);
    access_backend->flush_on(flush_level_from_config(logs, "access", spdlog::level::warn));
    
    auto error_backend = std::make_shared<spdlog::logger>(server_name + "_error", error_sinks.begin(), error_sinks.end());
    error_backend->set_level(spdlog::level::info);
    error_backend->flush_on(flush_level_from_config(logs, "error", spdlog::level::info));
    if (logs.contains("error") && logs.at("error").is_object())
    {
      const auto& error_cfg = logs.at("error");
      if (error_cfg.contains("level"))
      error_backend->set_level(spdlog::level::from_str(error_cfg.value("level", "info")));
    }
    
    if (!spdlog::get(access_backend->name()))
    spdlog::register_logger(access_backend);
    if (!spdlog::get(error_backend->name()))
    spdlog::register_logger(error_backend);
    
    return std::make_shared<f16::http::server::spdlog_logger>(access_backend, error_backend);
  }
  

static void build_simple_server(asio::io_context& ioc, f16::http::server::server_collection& servers, const std::string& root_doc, const std::string& bind_address, int port)
{
  auto spdlog_instance = spdlog::default_logger();
  auto log = std::make_shared<spdlog_logger>(spdlog_instance);  

  spdlog::info("Serving root doc {} on {}:{}", root_doc, bind_address, port);

  auto server = std::make_unique<http_server>(ioc, log);

  path_router router;
  router.add("/", static_content(root_doc));
  server->set(std::move(router));
  server->listen(std::to_string(port), bind_address);

  servers.server_set.push_back(std::move(server));
}

static void build_advanced_server(asio::io_context& ioc, f16::http::server::server_collection& servers, const std::string& cfg_file)
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
    const std::string server_name = address + '_' + port + (has_ssl ? "_ssl" : "_plain");
    
    // Create per-server logger using configuration
    logger_ptr server_logger = create_logger_from_config(server_name, jcfg);
    
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
        for (const std::string protocol : protocols)
          ssl_s.protocols.insert(protocol_from_string(protocol));
      }

      ssl_s.ciphers = ssl_section.value("ciphers", "");
      ssl_s.prefer_server_ciphers = ssl_section.value("prefer_server_ciphers", false);

      if (ssl_section.value("verify_client", false))
        ssl_s.client_certificate = ssl_section.value("client_certificate", "");

      ssl_s.session_timeout_secs = ssl_section.value("session_timeout_secs", -1L);
      ssl_s.session_cache = ssl_section.value("session_cache", false);
      ssl_s.session_cache_size = ssl_section.value("session_cache_size", -1L);

      server = std::make_unique<https_server>(ioc, ssl_s, server_logger);
    }
    else
      server = std::make_unique<http_server>(ioc, server_logger);

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
            for (const auto& [k, v] : header_entry.items())
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
                res = reply("Missing 'Host' header in the request", reply::bad_request, mime_types::extension_to_type(".txt"));
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
            res.add_header(name, value);
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
    
    servers.server_set.push_back(std::move(server));
  }
}

#endif

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
    config_cmd->add_option("config_path", config_path, "Configuration file path.")
      ->required()
      ->check(CLI::ExistingFile);

    app.parse(argc, argv);

    // http server
    asio::io_context ioc;

    std::chrono::milliseconds flush_interval{1000};

    nlohmann::json jcfg = nlohmann::json::object();
    if (serve_cmd->parsed())
    {
      // build jcfg from the command line arguments, to be passed to server_collection constructor
      jcfg["servers"] = nlohmann::json::array();
      nlohmann::json server_entry = nlohmann::json::object();
      server_entry["listen_address"] = bind_address;
      server_entry["listen_port"] = std::to_string(port);
      server_entry["locations"] = nlohmann::json::array();
      nlohmann::json location_entry = nlohmann::json::object();
      location_entry["location"] = "/";
      location_entry["root"] = root_doc;
      server_entry["locations"].push_back(location_entry);
      jcfg["servers"].push_back(server_entry);
    }
    else if (config_cmd->parsed())
    {
      // build jcfg from the config file, to be passed to server_collection constructor
      std::ifstream ifs(config_path);
      if (!ifs)
        throw std::runtime_error{ "Configuration file " + config_path + " not found" };
      jcfg = nlohmann::json::parse(ifs,
        nullptr, // callback
        true, // allow exceptions
        true // ignore_comments
      );
      flush_interval = flush_interval_from_config(jcfg);
    }
    else
    {
      fmt::print("{}", app.help());
      return 1;
    }
    
    logger_collection loggers(jcfg);

    // Keep log files aligned with low overhead.
    // Flushes all registered loggers approximately every flush_interval.
    spdlog::flush_every(flush_interval);

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
    
    server_collection servers(ioc, jcfg, loggers); // to keep the server instances alive

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
        spdlog::error("Exception in io_context scheduler: {}", e.what());
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
  catch (...)
  {
    fmt::print(stderr, "Unhandled unknown exception in main");
    return 1;
  }

  return 0;
}
