// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <asio.hpp> // NB: the asio header must be included *before* iostream to avoid sanity check error
#include <functional>
#include <fstream>
#include <iostream>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include "server.hpp"

#include "static_content.hpp"

// This file will be generated automatically when you run the CMake configuration step.
// It creates a namespace called `f16`.
// You can modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

static constexpr auto USAGE =
  R"(F16.

    Usage:
          f16 simple <root_doc> [--bind=<address> --port=<port>]
          f16 advanced <cfg_file>
          f16 (-h | --help)
          f16 --version

    Options:
          -h --help         Show this screen.
          --version         Show version.
          --bind=<address>  The binding address [default: 0.0.0.0].
          --port=<port>     The port [default: 80].
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

    std::vector<std::unique_ptr<server>> server_set;

    if (args["simple"].asBool())
    {
      const std::string address = args["--bind"].asString();
      const std::string port = args["--port"].asString();

      const std::string root_doc = args["<root_doc>"].asString();
      spdlog::info("Serving root doc {} on {}:{}", root_doc, address, port);

      auto http_server = std::make_unique<server>(ioc);

      http_server->add("/", std::make_shared<static_content>(root_doc));
      http_server->listen(port, address);

      server_set.push_back(std::move(http_server));
    }
    else if (args["advanced"].asBool())
    {
      const std::string cfg_file = args["<cfg_file>"].asString();
      std::ifstream ifs(cfg_file);
      if (!ifs) throw std::runtime_error("Configuration file " + cfg_file + " not fouund");
      const nlohmann::json cfg = nlohmann::json::parse(ifs,
        nullptr, // callback
        true, // allow exceptions
        true // ignore_comments
      );

      const std::string address = cfg.at("address");
      const std::string port = cfg.at("port");
      const std::string root_doc = cfg.at("root_doc");
      const std::string path = cfg.at("path");
      spdlog::info("Serving root doc {} on {}:{} path: {}", root_doc, address, port, path);

      auto http_server = std::make_unique<server>(ioc);
      http_server->add(path, std::make_shared<static_content>(root_doc));
      http_server->listen(port, address);
      server_set.push_back(std::move(http_server));
    }
      
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
    fmt::print("Unhandled exception in main: {}", e.what());
  }
}
