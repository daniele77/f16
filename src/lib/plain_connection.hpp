// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_PLAIN_CONNECTION_HPP
#define F16_HTTP_PLAIN_CONNECTION_HPP

#include <array>
#include <memory>
#include "f16asio.hpp"
#include "reply.hpp"
#include "http_request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "connection.hpp"

namespace f16::http::server {

class connection_manager;

/// Represents a single plain_connection from a client.
class plain_connection
  : public connection,
    public std::enable_shared_from_this<plain_connection>
{
public:
  plain_connection(const plain_connection&) = delete;
  plain_connection& operator=(const plain_connection&) = delete;

  /// Construct a plain_connection with the given socket.
  explicit plain_connection(asio::ip::tcp::socket socket,
      connection_manager& manager, request_handler& handler);

  /// Start the first asynchronous operation for the plain_connection.
  void start() override;

  /// Stop all asynchronous operations associated with the plain_connection.
  void stop() override;

private:
  /// Perform an asynchronous read operation.
  void do_read();

  /// Perform an asynchronous write operation.
  void do_write();

  /// Socket for the plain_connection.
  asio::ip::tcp::socket socket_;

  /// The manager for this plain_connection.
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

} // namespace f16::http::server

#endif // F16_HTTP_PLAIN_CONNECTION_HPP
