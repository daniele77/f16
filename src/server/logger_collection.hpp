// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_SERVER_LOGGER_COLLECTION_HPP
#define F16_HTTP_SERVER_LOGGER_COLLECTION_HPP

#include <nlohmann/json_fwd.hpp>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <vector>
#include "logger.hpp" // for logger_ptr

namespace f16::http::server {

/// Collection of logger instances
class logger_collection
{
public:
  logger_collection(const nlohmann::json& config);
  logger_ptr create_logger_from_config(const std::string& server_name) const;
private:
  spdlog::level::level_enum flush_level_from_config(const nlohmann::json& logger_config, spdlog::level::level_enum default_level) const;
  std::vector<spdlog::sink_ptr> create_sinks_from_config(const nlohmann::json& section) const;
  std::shared_ptr<spdlog::logger> create_backend_from_config(const nlohmann::json& logger_config, const std::string& server_name = "") const;

  const nlohmann::json& root_config;
  std::shared_ptr<spdlog::logger> null_backend;
  std::shared_ptr<spdlog::logger> default_backend;
};

} // namespace f16::http::server

#endif // F16_HTTP_SERVER_LOGGER_COLLECTION_HPP
