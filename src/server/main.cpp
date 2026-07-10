// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "f16/f16asio.hpp" // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
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
