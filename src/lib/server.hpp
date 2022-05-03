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

namespace f16 {
namespace http {
namespace server {

/// The top-level class of the HTTP server.
class server
{
public:
  server(const server&) = delete;
  server& operator=(const server&) = delete;

  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  /// For IPv4, try address: 0.0.0.0
  /// For IPv6, try address: 0::0
  explicit server(asio::io_context& ioc, const std::string& address, const std::string& port,
      const std::string& doc_root);

  /// Run the server's io_context loop.
  void run();

  void add_handler(const std::string& url, const std::function<void(std::ostream&)>& h);

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

} // namespace server
} // namespace http
} // namespace f16

#endif // F16_HTTP_SERVER_HPP