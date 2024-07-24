// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_HTTPS_SERVER_HPP
#define F16_HTTP_HTTPS_SERVER_HPP

#include <asio/ssl.hpp>
#include "http_server.hpp"

namespace f16::http::server {

// forward declaration
class http_handler;


/// The top-level class of the HTTPS server.
class https_server : public http_server
{
public:
  https_server(const https_server&) = delete;
  https_server& operator=(const https_server&) = delete;

  /// Construct the server
  explicit https_server(asio::io_context& ioc);

protected:

  connection_ptr create_connection(asio::ip::tcp::socket socket, connection_manager& cm, request_handler& rh) override;

private:

  asio::ssl::context ssl_context_;
};

} // namespace f16::http::server

#endif // F16_HTTP_HTTPS_SERVER_HPP
