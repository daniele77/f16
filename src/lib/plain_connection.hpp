// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_PLAIN_CONNECTION_HPP
#define F16_HTTP_PLAIN_CONNECTION_HPP

#include "f16asio.hpp"
#include "base_connection.hpp"

namespace f16::http::server {

class connection_manager;

/// Represents a single plain_connection from a client.
class plain_connection
  : public base_connection<asio::basic_stream_socket<asio::ip::tcp>>
{
public:
  plain_connection(const plain_connection&) = delete;
  plain_connection& operator=(const plain_connection&) = delete;

  /// Construct a plain_connection with the given socket.
  explicit plain_connection(asio::ip::tcp::socket socket,
      connection_manager& manager, request_handler& handler);

  void start() override;
};

} // namespace f16::http::server

#endif // F16_HTTP_PLAIN_CONNECTION_HPP
