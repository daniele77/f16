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
    asio::ssl::context& ctx, logger_ptr log, server_options options)
  : base_connection({std::move(socket), ctx}, manager, handler, std::move(log), std::move(options))
  , handshake_timer_(socket_.get_executor())
  , handshake_timeout_triggered_(false)
{
}

void ssl_connection::start()
{
  do_handshake();
}

void ssl_connection::do_handshake()
{
  handshake_timeout_triggered_ = false;

  auto self{this->shared_from_this()};

  handshake_timer_.expires_after(options_.tls_handshake_timeout);
  handshake_timer_.async_wait([this, self](const std::error_code& ec)
    {
      if (ec == asio::error::operation_aborted)
        return;

      handshake_timeout_triggered_ = true;
      log_->warn("Request rejected (tls-handshake-timeout)");
      connection_manager_.stop(self);
    });

  socket_.async_handshake(asio::ssl::stream_base::server,
      [this, self](std::error_code ec)
      {
        asio::error_code ignored_ec;
        handshake_timer_.cancel(ignored_ec);

        if (handshake_timeout_triggered_)
          return;

        if (!ec)
        {
          log_->debug("TLS handshake completed");
          do_read();
        }
        else
        {
          log_->warn("Request rejected (tls-handshake-failed): " + ec.message());
          connection_manager_.stop(self);
        }
      });
}

} // namespace f16::http::server
