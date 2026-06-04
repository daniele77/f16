// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "logger_collection.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#if defined(__unix__) || defined(__APPLE__)
#include <spdlog/sinks/syslog_sink.h>
#include <syslog.h>
#endif
#include "spdlog_logger.hpp"

namespace f16::http::server {

namespace {

#if defined(__unix__) || defined(__APPLE__)
int syslog_facility_from_string(const std::string& facility)
{
  if (facility == "auth") return LOG_AUTH;
#if defined(LOG_AUTHPRIV)
  if (facility == "authpriv") return LOG_AUTHPRIV;
#endif
  if (facility == "cron") return LOG_CRON;
  if (facility == "daemon") return LOG_DAEMON;
#if defined(LOG_FTP)
  if (facility == "ftp") return LOG_FTP;
#endif
  if (facility == "kern") return LOG_KERN;
  if (facility == "local0") return LOG_LOCAL0;
  if (facility == "local1") return LOG_LOCAL1;
  if (facility == "local2") return LOG_LOCAL2;
  if (facility == "local3") return LOG_LOCAL3;
  if (facility == "local4") return LOG_LOCAL4;
  if (facility == "local5") return LOG_LOCAL5;
  if (facility == "local6") return LOG_LOCAL6;
  if (facility == "local7") return LOG_LOCAL7;
  if (facility == "lpr") return LOG_LPR;
  if (facility == "mail") return LOG_MAIL;
  if (facility == "news") return LOG_NEWS;
  if (facility == "syslog") return LOG_SYSLOG;
  if (facility == "user") return LOG_USER;
  if (facility == "uucp") return LOG_UUCP;
  throw std::invalid_argument("Unknown syslog facility: " + facility);
}

int parse_syslog_facility(const nlohmann::json& sink_cfg)
{
  if (!sink_cfg.contains("facility"))
    return LOG_USER;

  const auto& facility_cfg = sink_cfg.at("facility");
  if (facility_cfg.is_number_integer())
    return facility_cfg.get<int>();
  if (facility_cfg.is_string())
    return syslog_facility_from_string(facility_cfg.get<std::string>());

  throw std::invalid_argument("syslog 'facility' must be an integer or a string");
}

int syslog_option_from_string(const std::string& option)
{
  if (option == "pid") return LOG_PID;
  if (option == "cons") return LOG_CONS;
#if defined(LOG_NDELAY)
  if (option == "ndelay") return LOG_NDELAY;
#endif
#if defined(LOG_ODELAY)
  if (option == "odelay") return LOG_ODELAY;
#endif
#if defined(LOG_NOWAIT)
  if (option == "nowait") return LOG_NOWAIT;
#endif
#if defined(LOG_PERROR)
  if (option == "perror") return LOG_PERROR;
#endif
  throw std::invalid_argument("Unknown syslog option: " + option);
}

int parse_syslog_option(const nlohmann::json& sink_cfg)
{
  if (!sink_cfg.contains("option") && !sink_cfg.contains("options"))
    return LOG_PID;

  if (sink_cfg.contains("option"))
  {
    const auto& option_cfg = sink_cfg.at("option");
    if (option_cfg.is_number_integer())
      return option_cfg.get<int>();
    if (option_cfg.is_string())
      return syslog_option_from_string(option_cfg.get<std::string>());
    throw std::invalid_argument("syslog 'option' must be an integer or a string");
  }

  int option_flags = 0;
  const auto& options_cfg = sink_cfg.at("options");
  if (!options_cfg.is_array())
    throw std::invalid_argument("syslog 'options' must be an array");

  for (const auto& entry : options_cfg)
  {
    if (entry.is_number_integer())
    {
      option_flags |= entry.get<int>();
      continue;
    }
    if (entry.is_string())
    {
      option_flags |= syslog_option_from_string(entry.get<std::string>());
      continue;
    }
    throw std::invalid_argument("syslog 'options' entries must be integers or strings");
  }
  return option_flags;
}
#endif

} // namespace

logger_collection::logger_collection(const nlohmann::json& config) :
  root_config(config),
  null_backend(std::make_shared<spdlog::logger>(
    "null_logger",
    std::make_shared<spdlog::sinks::null_sink_mt>()
  )),
  default_backend(null_backend)
{
  // read the section "error" of "logs" and setup the default logger accordingly, to be used as default logger for all servers
  if (config.contains("logs") && config.at("logs").is_object())
  {
    const auto& logs = config.at("logs");
    if (logs.contains("error") && logs.at("error").is_object())
    {
      const auto& error_log_config = logs.at("error");
      default_backend = create_backend_from_config(error_log_config);
      assert(default_backend != nullptr);
      spdlog::set_default_logger(default_backend);
    }
  }
}

// create a f16::http::server::logger for a server based on the configuration, or return nullptr if no logger is configured for that server
logger_ptr logger_collection::create_logger_from_config(const std::string& server_name) const
{
  nlohmann::json logs = nlohmann::json::object();
  if (root_config.contains("logs") && root_config.at("logs").is_object())
  {
    logs = root_config.at("logs");
  }
  
  if (logs.contains("access") && logs.at("access").is_object())
  {
    const auto& access_log_config = logs.at("access");
    auto access_backend = logger_collection::create_backend_from_config(access_log_config, server_name);
    assert(access_backend != nullptr);
    access_backend->set_pattern("%Y-%m-%d %H:%M:%S.%e | %n | %v");
    if (!spdlog::get(access_backend->name()))
      spdlog::register_logger(access_backend);
    return std::make_shared<f16::http::server::spdlog_logger>(access_backend, default_backend);
  }
  
  return std::make_shared<f16::http::server::spdlog_logger>(null_backend, default_backend);
}

spdlog::level::level_enum logger_collection::flush_level_from_config(const nlohmann::json& logger_config, spdlog::level::level_enum default_level) const
{
  if (!logger_config.contains("flush_on_level"))
    return default_level;
    
  const std::string level_s = logger_config.value("flush_on_level", "");
  if (level_s.empty())
    return default_level;
    
  return spdlog::level::from_str(level_s);
}

std::vector<spdlog::sink_ptr> logger_collection::create_sinks_from_config(const nlohmann::json& logger_config) const
{
  std::vector<spdlog::sink_ptr> sinks;
    
  if (!logger_config.contains("sinks") || !logger_config.at("sinks").is_array())
    return sinks;
    
  for (const auto& sink_cfg : logger_config.at("sinks"))
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
      #if defined(__unix__) || defined(__APPLE__)
        const std::string ident = sink_cfg.value("ident", std::string("f16"));
        const int option = parse_syslog_option(sink_cfg);
        const int facility = parse_syslog_facility(sink_cfg);
        const bool enable_formatting = sink_cfg.value("enable_formatting", false);
        sink = std::make_shared<spdlog::sinks::syslog_sink_mt>(ident, option, facility, enable_formatting);
      #else
        spdlog::warn("syslog sink is not supported on this platform; skipping");
        continue;
      #endif
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
      spdlog::warn("Failed to create sink of type '{}': {}", type, e.what()); // TODO
    }
  }
    
  return sinks;
}

// create a spdlog::logger based on the configuration of a server, or return nullptr if no logger is configured for that server
std::shared_ptr<spdlog::logger> logger_collection::create_backend_from_config(const nlohmann::json& logger_config, const std::string& server_name) const
{
  const bool enabled = logger_config.value("enabled", true);
  if (enabled)
  {
    auto sinks = create_sinks_from_config(logger_config);
    if (sinks.empty())
      sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
    auto backend = std::make_shared<spdlog::logger>(server_name, sinks.begin(), sinks.end());
    backend->set_level(spdlog::level::info);
    backend->flush_on(flush_level_from_config(logger_config, spdlog::level::info));
    if (logger_config.contains("level") && logger_config.at("level").is_string())
    {
      const std::string level_s = logger_config.value("level", "info");
      backend->set_level(spdlog::level::from_str(level_s));
    }

    if (!spdlog::get(backend->name()))
      spdlog::register_logger(backend);
    
    return backend;
  }

  return null_backend;
}

} // namespace f16::http::server
