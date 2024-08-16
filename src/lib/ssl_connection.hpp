// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_SSL_CONNECTION_HPP
#define F16_HTTP_SSL_CONNECTION_HPP

#include "f16asio.hpp"
#include <asio/ssl.hpp>
#include "base_connection.hpp"

namespace f16::http::server {

class connection_manager;

/// Represents a single connection from a client.
class ssl_connection
  : public base_connection<asio::ssl::stream<asio::ip::tcp::socket>>
{
public:
  ssl_connection(const ssl_connection&) = delete;
  ssl_connection& operator=(const ssl_connection&) = delete;

  /// Construct a connection with the given socket.
  explicit ssl_connection(asio::ip::tcp::socket socket,
      connection_manager& manager, request_handler& handler,
      asio::ssl::context& ctx);

  void start() override;

protected:

  void do_handshake();
};

} // namespace f16::http::server

#endif // F16_HTTP_SSL_CONNECTION_HPP
