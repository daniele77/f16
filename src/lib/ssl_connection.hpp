// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_SSL_CONNECTION_HPP
#define F16_HTTP_SSL_CONNECTION_HPP

#include <array>
#include <memory>
#include "f16asio.hpp"
#include <asio/ssl.hpp>
#include "reply.hpp"
#include "http_request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace f16::http::server {

class connection_manager;

/// Represents a single connection from a client.
class ssl_connection
  : public std::enable_shared_from_this<ssl_connection>
{
public:
  ssl_connection(const ssl_connection&) = delete;
  ssl_connection& operator=(const ssl_connection&) = delete;

  /// Construct a connection with the given socket.
  explicit ssl_connection(asio::ip::tcp::socket socket,
      connection_manager& manager, request_handler& handler,
      asio::ssl::context& ctx);

  /// Start the first asynchronous operation for the connection.
  void start();

  /// Stop all asynchronous operations associated with the connection.
  void stop();

private:

  void do_handshake();

  /// Perform an asynchronous read operation.
  void do_read();

  /// Perform an asynchronous write operation.
  void do_write();

  /// SSL socket for the connection.
  asio::ssl::stream<asio::ip::tcp::socket> socket_;

  /// The manager for this connection.
  connection_manager& connection_manager_;

  /// The handler used to process the incoming request.
  request_handler& request_handler_;

  /// Buffer for incoming data.
  std::array<char, 8192> buffer_;

  /// The incoming request.
  http_request request_;

  /// The parser for the incoming request.
  request_parser request_parser_;

  /// The reply to be sent back to the client.
  reply reply_;
};

using ssl_connection_ptr = std::shared_ptr<ssl_connection>;

} // namespace f16::http::server

#endif // F16_HTTP_SSL_CONNECTION_HPP
