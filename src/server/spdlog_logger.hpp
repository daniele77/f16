// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_SERVER_SPDLOG_LOGGER_HPP
#define F16_HTTP_SERVER_SPDLOG_LOGGER_HPP

#include "../lib/logger.hpp"
#include <spdlog/spdlog.h>
#include <memory>

namespace f16::http::server {

/// Logger implementation using spdlog
class spdlog_logger : public logger
{
public:
  /// Create a logger with the given spdlog logger
  explicit spdlog_logger(std::shared_ptr<spdlog::logger> spdlog_instance)
    : spdlog_logger_(spdlog_instance)
  {
  }

  void access(const access_log_event& event) override
  {
    spdlog_logger_->info("{} {} {} {} {} \"{}\"",
      event.client_ip,
      event.method,
      event.uri,
      event.status_code,
      event.response_size,
      event.user_agent);
  }

  void info(const std::string& message) override
  {
    spdlog_logger_->info(message);
  }

  void warn(const std::string& message) override
  {
    spdlog_logger_->warn(message);
  }

  void error(const std::string& message) override
  {
    spdlog_logger_->error(message);
  }

  void debug(const std::string& message) override
  {
    spdlog_logger_->debug(message);
  }

private:
  std::shared_ptr<spdlog::logger> spdlog_logger_;
};

} // namespace f16::http::server

#endif // F16_HTTP_SERVER_SPDLOG_LOGGER_HPP
