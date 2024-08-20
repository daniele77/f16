// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_HTTPS_SERVER_HPP
#define F16_HTTP_HTTPS_SERVER_HPP

#include <asio/ssl.hpp>
#include <unordered_set>
#include "http_server.hpp"

namespace f16::http::server {

// forward declaration
class http_handler;


struct ssl_settings
{
  enum ssl_proto
  {
    sslv2 = asio::ssl::context::sslv2,
    sslv3 = asio::ssl::context::sslv3,
    tlsv1 = asio::ssl::context::tlsv1,
    tlsv11 = asio::ssl::context::tlsv11,
    tlsv12 = asio::ssl::context::tlsv12,
    tlsv13 = asio::ssl::context::tlsv13
  };
  std::string certificate = {};
  std::string certificate_key = {};
  std::string dhparam = {};
  std::string password = {};
  std::unordered_set<ssl_proto> protocols = { tlsv1, tlsv11, tlsv12, tlsv13 };
  std::string ciphers = {};
  std::string client_certificate = {};
};


/// The top-level class of the HTTPS server.
class https_server : public http_server
{
public:
  https_server(const https_server&) = delete;
  https_server& operator=(const https_server&) = delete;

  /// Construct the server
  https_server(asio::io_context& ioc, const ssl_settings& ssl_s);

protected:

  connection_ptr create_connection(asio::ip::tcp::socket socket, connection_manager& cm, request_handler& rh) override;

private:

  asio::ssl::context ssl_context_;
};

} // namespace f16::http::server

#endif // F16_HTTP_HTTPS_SERVER_HPP
