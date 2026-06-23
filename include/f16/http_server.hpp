// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_HTTP_SERVER_HPP
#define F16_HTTP_HTTP_SERVER_HPP

#include "f16asio.hpp"
#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include "logger.hpp"

namespace f16::http::server {

class connection;
using connection_ptr = std::shared_ptr<connection>;

class connection_manager;
class request_handler;

struct http_request;
struct reply;

struct server_options
{
  std::size_t max_request_line_bytes = 8192;
  std::size_t max_header_section_bytes = 32768;
  std::size_t max_headers_count = 100;
  std::size_t max_body_bytes = 1048576;
  std::chrono::milliseconds read_header_timeout = std::chrono::seconds(15);
  std::chrono::milliseconds read_body_timeout = std::chrono::seconds(30);
  std::chrono::milliseconds tls_handshake_timeout = std::chrono::seconds(10);
};

/// The top-level class of the HTTP server.
class http_server
{
public:
  using handler_fn = std::function<void(const http_request& req, reply& rep)>;

  http_server(const http_server&) = delete;
  http_server& operator=(const http_server&) = delete;

  /// Construct the server with optional logger
  explicit http_server(asio::io_context& ioc, logger_ptr log = nullptr);

  /// Construct the server with explicit options and optional logger
  http_server(asio::io_context& ioc, server_options options, logger_ptr log = nullptr);

  /// Cancel all outstanding asynchronous operations.
  /// Once all operations have finished the destructor will exit.

  virtual ~http_server();

  void set(handler_fn handler);

  /// Start to listen on the specified TCP address and port
  /// For IPv4, try address: 0.0.0.0
  /// For IPv6, try address: 0::0
  void listen(const std::string& port = "80", const std::string& address = "0.0.0.0");

protected:

  /// Get the logger
  logger_ptr get_logger() const { return log_; }
  const server_options& get_options() const { return options_; }

  virtual connection_ptr create_connection(asio::ip::tcp::socket socket, connection_manager& cm, request_handler& rh);
  virtual std::string protocol_name() const { return "HTTP"; }

private:
  /// Perform an asynchronous accept operation.
  void do_accept();
  
  /// The io_context used to perform asynchronous operations.
  asio::io_context& io_context_;

  /// Acceptor used to listen for incoming connections.
  asio::ip::tcp::acceptor acceptor_;

  /// The connection manager which owns all live connections.
  std::unique_ptr<connection_manager> connection_manager_;

  /// The handler for all incoming requests.
  std::unique_ptr<request_handler> request_handler_;

  /// Logger instance
  logger_ptr log_;

  /// Server runtime options
  server_options options_;
};

} // namespace f16::http::server

#endif // F16_HTTP_HTTP_SERVER_HPP
