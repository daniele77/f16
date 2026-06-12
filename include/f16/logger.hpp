// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_LOGGER_HPP
#define F16_HTTP_LOGGER_HPP

#include <memory>
#include <string>

namespace f16::http::server {

struct access_log_event
{
  std::string client_ip;
  std::string method;
  std::string uri;
  int status_code;
  std::size_t response_size;
  std::string user_agent;
};

/// Abstract logger interface for f16 library
/// Users of the library can implement this interface with their preferred logging backend
class logger
{
public:
  virtual ~logger() = default;

  /// Log an access message (e.g. for HTTP requests)
  virtual void access(const access_log_event& event) = 0;

  /// Log an informational message
  virtual void info(const std::string& message) = 0;

  /// Log a warning message
  virtual void warn(const std::string& message) = 0;

  /// Log an error message
  virtual void error(const std::string& message) = 0;

  /// Log a debug message
  virtual void debug(const std::string& message) = 0;
};

using logger_ptr = std::shared_ptr<logger>;

/// Default null logger that does nothing
/// Used when no logger is provided
class null_logger : public logger
{
public:
  void access(const access_log_event&) override {}
  void info(const std::string&) override {}
  void warn(const std::string&) override {}
  void error(const std::string&) override {}
  void debug(const std::string&) override {}
};

} // namespace f16::http::server

#endif // F16_HTTP_LOGGER_HPP
