// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_HTTP_SERVER_HPP
#define F16_HTTP_HTTP_SERVER_HPP

#include "f16asio.hpp"
#include <string>
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace f16::http::server {

// forward declaration
class http_handler;


/// The top-level class of the HTTP server.
class http_server
{
public:
  http_server(const http_server&) = delete;
  http_server& operator=(const http_server&) = delete;

  /// Construct the server
  explicit http_server(asio::io_context& ioc);

  /// Cancel all outstanding asynchronous operations.
  /// Once all operations have finished the destructor will exit.

  virtual ~http_server();

#ifndef NEW_CODE
  void set(path_router handler);
#endif
  void set(handler_fn handler);

  // void set_handler(const std::function<void(const http_request& req, reply& rep)> handler);

  /// Start to listen on the specified TCP address and port
  /// For IPv4, try address: 0.0.0.0
  /// For IPv6, try address: 0::0
  void listen(const std::string& port = "80", const std::string& address = "0.0.0.0");

protected:

  virtual connection_ptr create_connection(asio::ip::tcp::socket socket, connection_manager& cm, request_handler& rh);

private:
  /// Perform an asynchronous accept operation.
  void do_accept();
  
  /// The io_context used to perform asynchronous operations.
  asio::io_context& io_context_;

  /// Acceptor used to listen for incoming connections.
  asio::ip::tcp::acceptor acceptor_;

  /// The connection manager which owns all live connections.
  connection_manager connection_manager_;

  /// The handler for all incoming requests.
  request_handler request_handler_;
};

} // namespace f16::http::server

#endif // F16_HTTP_HTTP_SERVER_HPP
