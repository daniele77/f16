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
  : socket_(std::move(socket), ctx),
    connection_manager_(manager),
    request_handler_(handler),
    buffer_{},
    request_{},
    reply_{}
{
}

void ssl_connection::start()
{
  do_handshake();
}

void ssl_connection::do_handshake()
{
  auto self(shared_from_this());
  socket_.async_handshake(asio::ssl::stream_base::server,
      [self](std::error_code ec)
      {
        if (!ec)
        {
          self->do_read();
        }
        else
        {
          self->connection_manager_.stop(self);
        }
      });
}

void ssl_connection::stop()
{
  socket_.lowest_layer().close();
}

void ssl_connection::do_read()
{
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer_),
      [this, self](std::error_code ec, std::size_t bytes_transferred)
      {
        if (!ec)
        {
          request_parser::result_type result = request_parser::bad;
          std::tie(result, std::ignore) = request_parser_.parse(
              request_, buffer_.begin(), buffer_.begin() + bytes_transferred);

          if (result == request_parser::good)
          {
            request_handler_.handle_request(request_, reply_);
            do_write();
          }
          else if (result == request_parser::bad)
          {
            reply_ = reply::stock_reply(reply::bad_request);
            do_write();
          }
          else
          {
            do_read();
          }
        }
        else if (ec != asio::error::operation_aborted)
        {
          connection_manager_.stop(shared_from_this());
        }
      });
}

void ssl_connection::do_write()
{
  auto self(shared_from_this());
  asio::async_write(socket_, reply_.to_buffers(),
      [this, self](std::error_code ec, std::size_t)
      {
        if (!ec)
        {
          // Initiate graceful connection closure.
          asio::error_code ignored_ec;
          socket_.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
        }

        if (ec != asio::error::operation_aborted)
        {
          connection_manager_.stop(shared_from_this());
        }
      });
}

} // namespace f16::http::server
