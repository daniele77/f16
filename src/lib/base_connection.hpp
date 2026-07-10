// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_BASE_CONNECTION_HPP
#define F16_HTTP_BASE_CONNECTION_HPP

#include <array>
#include <chrono>
#include "connection_manager.hpp"
#include "f16/http_server.hpp"
#include "f16/http_request.hpp"
#include "request_parser.hpp"
#include "request_handler.hpp"
#include "f16/reply.hpp"
#include "connection.hpp"
#include "f16/logger.hpp"

namespace f16::http::server {

template <typename SocketType>
class base_connection
  : public connection,
    public std::enable_shared_from_this<base_connection<SocketType>>
{
public:

  void stop() override
  {
    cancel_read_timeout();
    socket_.lowest_layer().close();
  }

protected:

  base_connection(SocketType socket, connection_manager& manager, request_handler& handler, logger_ptr log, server_options options)
    : socket_(std::move(socket)),
      connection_manager_(manager),
      request_handler_(handler),
      log_(log),
      options_(std::move(options)),
      read_timer_(socket_.get_executor()),
      read_timeout_triggered_(false),
      buffer_{},
      request_{},
      request_parser_({
        options_.max_request_line_bytes,
        options_.max_header_section_bytes,
        options_.max_headers_count,
        options_.max_body_bytes
      }),
      reply_{}
  {
  }

  void do_read()
  {
    arm_read_timeout();
    auto self{this->shared_from_this()};
    socket_.async_read_some(asio::buffer(buffer_),
        [this, self](std::error_code ec, std::size_t bytes_transferred)
        {
          cancel_read_timeout();

          if (read_timeout_triggered_)
            return;

          if (!ec)
          {
            // Populate client IP from socket (once per read cycle)
            if (request_.client_ip.empty())
            {
              try
              {
                request_.client_ip = socket_.lowest_layer().remote_endpoint().address().to_string();
              }
              catch (...)
              {
                request_.client_ip = "unknown";
              }
            }

            request_parser::result_type result = request_parser::bad;
            std::tie(result, std::ignore) = request_parser_.parse(
                request_, buffer_.begin(), buffer_.begin() + bytes_transferred);

            if (result == request_parser::good)
            {
              request_handler_.handle_request(request_, reply_);

              // Emit access log
              log_->access({
                request_.client_ip,
                request_.method,
                request_.uri,
                static_cast<int>(reply_.status),
                reply_.content.size(),
                request_.get_header("user-agent")
              });

              do_write();
            }
            else if (result == request_parser::bad)
              reject_request(reply::bad_request, "bad-request");
            else if (result == request_parser::too_large)
              reject_request(reply::request_entity_too_large, "request-too-large");
            else
            {
              do_read();
            }
          }
          else if (ec != asio::error::operation_aborted)
          {
            log_->error("Read error from " + request_.client_ip + ": " + ec.message());
            connection_manager_.stop(this->shared_from_this());
          }
        });
  }

  void do_write()
  {
    cancel_read_timeout();
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
            if (ec)
              self->log_->warn("Write error: " + ec.message());
            self->connection_manager_.stop(self->shared_from_this());
          }
        });
  }

  void arm_read_timeout()
  {
    read_timer_.expires_after(current_read_timeout());
    auto self{this->shared_from_this()};
    read_timer_.async_wait([this, self](const std::error_code& ec)
      {
        if (ec == asio::error::operation_aborted)
          return;

        read_timeout_triggered_ = true;
        reject_request(reply::request_timeout, request_parser_.is_reading_body() ? "body-timeout" : "header-timeout");
      });
  }

  void cancel_read_timeout()
  {
    asio::error_code ignored_ec;
    read_timer_.cancel(ignored_ec);
  }

  std::chrono::milliseconds current_read_timeout() const
  {
    if (request_parser_.is_reading_body())
      return options_.read_body_timeout;
    return options_.read_header_timeout;
  }

  void reject_request(reply::status_type status, const char* reason)
  {
    reply_ = reply::stock_reply(status);

    const std::string client_ip = request_.client_ip.empty() ? "unknown" : request_.client_ip;
    const std::string method = request_.method.empty() ? "?" : request_.method;
    const std::string uri = request_.uri.empty() ? "?" : request_.uri;
    log_->warn(std::string("Request rejected (") + reason + ") from " + client_ip +
      " method=" + method + " uri=" + uri);

    do_write();
  }

  SocketType socket_;

  /// The manager for this connection.
  connection_manager& connection_manager_;

  /// The handler used to process the incoming request.
  request_handler& request_handler_;

  /// Logger
  logger_ptr log_;

  /// Runtime server options
  server_options options_;

  /// Timer used to enforce read timeouts
  asio::steady_timer read_timer_;

  /// Indicates that the read timeout was hit for this connection
  bool read_timeout_triggered_;

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
