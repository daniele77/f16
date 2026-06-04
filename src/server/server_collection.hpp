// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_SERVER_SERVER_COLLECTION_HPP
#define F16_HTTP_SERVER_SERVER_COLLECTION_HPP

#include <vector>
#include <memory>
#include "http_server.hpp"
#include <nlohmann/json_fwd.hpp>
#include "https_server.hpp" // For ssl_settings struct

namespace f16::http::server {

// forward declaration
class logger_collection;

/// Collection of HTTP server instances
class server_collection
{
public:
  server_collection(asio::io_context& ioc, const nlohmann::json& config, const logger_collection& logger_collection);

private:
  void add_server(asio::io_context& ioc, const nlohmann::json& server_cfg);
  void config_location_based_server(http_server& server, const nlohmann::json& locations);
  void config_return_based_server(http_server& server, const nlohmann::json& return_section);
  static ssl_settings get_ssl_settings(const nlohmann::json& ssl_cfg);
  static f16::http::server::ssl_settings::ssl_proto protocol_from_string(const std::string& s);

  const logger_collection& loggers;
  std::vector<std::unique_ptr<http_server>> server_set;
};

} // namespace f16::http::server

#endif // F16_HTTP_SERVER_SERVER_COLLECTION_HPP
