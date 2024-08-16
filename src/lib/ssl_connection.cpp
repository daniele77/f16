// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ssl_connection.hpp"
#include <utility>
#include <vector>
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace f16::http::server {

ssl_connection::ssl_connection(asio::ip::tcp::socket socket,
    connection_manager& manager, request_handler& handler,
    asio::ssl::context& ctx)
  : base_connection({std::move(socket), ctx}, manager, handler) 
{
}

void ssl_connection::start()
{
  do_handshake();
}

void ssl_connection::do_handshake()
{
  auto self{this->shared_from_this()};
  socket_.async_handshake(asio::ssl::stream_base::server,
      [this, self](std::error_code ec)
      {
        if (!ec)
        {
          do_read();
        }
        else
        {
          connection_manager_.stop(self);
        }
      });
}

} // namespace f16::http::server
