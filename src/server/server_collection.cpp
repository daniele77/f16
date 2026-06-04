// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "server_collection.hpp"
#include "http_server.hpp"
#include "https_server.hpp"
#include "http_request.hpp"
#include "reply.hpp"
#include "mime_types.hpp"
#include "logger_collection.hpp"

namespace f16::http::server {


f16::http::server::ssl_settings::ssl_proto server_collection::protocol_from_string(const std::string& s)
{
  if (s == "SSLv2") return ssl_settings::sslv2;
  if (s == "SSLv3") return ssl_settings::sslv3;
  if (s == "TLSv1") return ssl_settings::tlsv1;
  if (s == "TLSv1.1") return ssl_settings::tlsv11;
  if (s == "TLSv1.2") return ssl_settings::tlsv12;
  if (s == "TLSv1.3") return ssl_settings::tlsv13;
  throw std::invalid_argument("Unknown protocol: " + s);
}

server_collection::server_collection(asio::io_context& ioc, const nlohmann::json& config, const logger_collection& _loggers)
  : loggers(_loggers)
{
  for (const auto& server_cfg : config.at("servers"))
    add_server(ioc, server_cfg);
}

void server_collection::add_server(asio::io_context& ioc, const nlohmann::json& server_cfg)
{
  const std::string address = server_cfg.at("listen_address");
  const bool has_ssl = server_cfg.contains("ssl");
  const std::string port = server_cfg.value("listen_port", (has_ssl ? "443" : "80"));
  const std::string server_name = address + '_' + port + (has_ssl ? "_ssl" : "_plain");

  logger_ptr server_logger = loggers.create_logger_from_config(server_name);
  spdlog::info("New {} server listening on {}:{}", (has_ssl ? "https" : "http"), address, port);

  std::unique_ptr<http_server> server;
  if (has_ssl)
  {
    ssl_settings ssl_s = get_ssl_settings(server_cfg.at("ssl"));
    server = std::make_unique<https_server>(ioc, ssl_s, server_logger);
  }
  else
  {
    server = std::make_unique<http_server>(ioc, server_logger);
  }

  if (server_cfg.contains("return"))
  {
    const auto& return_section = server_cfg.at("return");
    config_return_based_server(*server, return_section);
  }
  else if (server_cfg.contains("locations"))
  {
    const auto& locations = server_cfg.at("locations");
    config_location_based_server(*server, locations);
  }
  else
  {
    spdlog::warn("No 'return' or 'locations' section found for this server entry: this server will not handle any request");
  }

  server->listen(port, address);  
  server_set.push_back(std::move(server));
}

ssl_settings server_collection::get_ssl_settings(const nlohmann::json& ssl_cfg)
{
  ssl_settings ssl_s;
  ssl_s.certificate = ssl_cfg.value("certificate", "");
  ssl_s.certificate_key = ssl_cfg.value("certificate_key", "");
  ssl_s.dhparam = ssl_cfg.value("dhparam", "");
  ssl_s.password = ssl_cfg.value("password", "");
  if (ssl_cfg.contains("protocols"))
  {
    for (std::string protocol : ssl_cfg.at("protocols"))
        ssl_s.protocols.insert(protocol_from_string(protocol));
  }
  ssl_s.ciphers = ssl_cfg.value("ciphers", "");
  ssl_s.prefer_server_ciphers = ssl_cfg.value("prefer_server_ciphers", false);
  if (ssl_cfg.value("verify_client", false))
    ssl_s.client_certificate = ssl_cfg.value("client_certificate", "");
  ssl_s.session_timeout_secs = ssl_cfg.value("session_timeout_secs", -1L);
  ssl_s.session_cache = ssl_cfg.value("session_cache", false);
  ssl_s.session_cache_size = ssl_cfg.value("session_cache_size", -1L);

  return ssl_s;
}

void server_collection::config_return_based_server(http_server& server, const nlohmann::json& return_section)
{
  const std::string status_s = return_section.value("status", "ok");
  const reply::status_type status = reply::status_from_string(status_s);
  const auto headers = return_section.value("headers", nlohmann::json::array());
  server.set(
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

void server_collection::config_location_based_server(http_server& server, const nlohmann::json& locations)
{
  path_router router;
  for (const auto& location_entry : locations)
  {
    std::string root_doc = location_entry.at("root");
    std::string path = location_entry.at("location");
    spdlog::info("  Serving root doc {} at path: {}", root_doc, path);
    router.add(path, static_content(root_doc));
  }
  server.set(std::move(router));
}

} // namespace f16::http::server
