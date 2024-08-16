// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_BASE_CONNECTION_HPP
#define F16_HTTP_BASE_CONNECTION_HPP

#include <array>
#include "connection_manager.hpp"
#include "http_request.hpp"
#include "request_parser.hpp"
#include "request_handler.hpp"
#include "reply.hpp"
#include "connection.hpp"

namespace f16::http::server {

template <typename SocketType>
class base_connection
  : public connection,
    public std::enable_shared_from_this<base_connection<SocketType>>
{
public:

  void stop() override
  {
    socket_.lowest_layer().close();
  }

protected:

  base_connection(SocketType socket, connection_manager& manager, request_handler& handler)
    : socket_(std::move(socket)),
      connection_manager_(manager),
      request_handler_(handler),
      buffer_{},
      request_{},
      reply_{}
  {
  }

  void do_read()
  {
    auto self{this->shared_from_this()};
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
            connection_manager_.stop(this->shared_from_this());
          }
        });
  }

  void do_write()
  {
    auto self{this->shared_from_this()};
    asio::async_write(socket_, reply_.to_buffers(),
        [self](std::error_code ec, std::size_t)
        {
          if (!ec)
          {
            // Initiate graceful connection closure.
            asio::error_code ignored_ec;
            self->socket_.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both,
              ignored_ec);
          }

          if (ec != asio::error::operation_aborted)
          {
            self->connection_manager_.stop(self->shared_from_this());
          }
        });
  }

  SocketType socket_;

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

} // namespace f16::http::server

#endif // F16_HTTP_BASE_CONNECTION_HPP
