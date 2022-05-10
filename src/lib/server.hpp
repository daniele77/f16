// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_SERVER_HPP
#define F16_HTTP_SERVER_HPP

#include <asio.hpp>
#include <string>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace f16::http::server {

// forward declaration
class http_handler;


/// The top-level class of the HTTP server.
class server
{
public:
  server(const server&) = delete;
  server& operator=(const server&) = delete;

  /// Construct the server
  explicit server(asio::io_context& ioc);

  void add(const std::string& path, std::shared_ptr<http_handler>);

  /// Start to listen on the specified TCP address and port
  /// For IPv4, try address: 0.0.0.0
  /// For IPv6, try address: 0::0
  void listen(const std::string& port = "80", const std::string& address = "0.0.0.0");

private:
  /// Perform an asynchronous accept operation.
  void do_accept();

  /// Wait for a request to stop the server.
  void do_await_stop();

  /// The io_context used to perform asynchronous operations.
  asio::io_context& io_context_;

  /// The signal_set is used to register for process termination notifications.
  asio::signal_set signals_;

  /// Acceptor used to listen for incoming connections.
  asio::ip::tcp::acceptor acceptor_;

  /// The connection manager which owns all live connections.
  connection_manager connection_manager_;

  /// The handler for all incoming requests.
  request_handler request_handler_;
};

} // namespace f16::http::server

#endif // F16_HTTP_SERVER_HPP
