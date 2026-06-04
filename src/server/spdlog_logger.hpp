// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_SERVER_SPDLOG_LOGGER_HPP
#define F16_HTTP_SERVER_SPDLOG_LOGGER_HPP

#include "logger.hpp"
#include <spdlog/spdlog.h>
#include <memory>
#include <cassert>

namespace f16::http::server {

/// Logger implementation using spdlog
class spdlog_logger : public logger
{
public:
  /// Create a logger with the same backend for access and error logs
  explicit spdlog_logger(std::shared_ptr<spdlog::logger> spdlog_instance)
    : access_logger_(std::move(spdlog_instance)),
      error_logger_(access_logger_)
  {
    assert(access_logger_ != nullptr);
    assert(error_logger_ != nullptr);
  }

  /// Create a logger with dedicated backends for access and error logs
  spdlog_logger(std::shared_ptr<spdlog::logger> access_logger, std::shared_ptr<spdlog::logger> error_logger)
    : access_logger_(std::move(access_logger)),
      error_logger_(std::move(error_logger))
  {
    assert(access_logger_ != nullptr);
    assert(error_logger_ != nullptr);
  }

  void access(const access_log_event& event) override
  {
    access_logger_->info("{} {} {} {} {} \"{}\"",
      event.client_ip,
      event.method,
      event.uri,
      event.status_code,
      event.response_size,
      event.user_agent);
  }

  void info(const std::string& message) override
  {
    error_logger_->info(message);
  }

  void warn(const std::string& message) override
  {
    error_logger_->warn(message);
  }

  void error(const std::string& message) override
  {
    error_logger_->error(message);
  }

  void debug(const std::string& message) override
  {
    error_logger_->debug(message);
  }

private:
  std::shared_ptr<spdlog::logger> access_logger_;
  std::shared_ptr<spdlog::logger> error_logger_;
};

} // namespace f16::http::server

#endif // F16_HTTP_SERVER_SPDLOG_LOGGER_HPP
