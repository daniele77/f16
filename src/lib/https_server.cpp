// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "https_server.hpp"
#include "ssl_connection.hpp"

namespace f16::http::server {

https_server::https_server(asio::io_context& ioc)
  : http_server(ioc),
    ssl_context_(asio::ssl::context::tlsv13)
{
  ssl_context_.set_options(
        asio::ssl::context::default_workarounds |
        asio::ssl::context::no_sslv2 |
        asio::ssl::context::single_dh_use
  );
  ssl_context_.set_password_callback([](std::size_t /*size*/, asio::ssl::context::password_purpose /*purpose*/) -> std::string { return "123456"; } ); // serve solo se serve decifrare la chiave privata
  ssl_context_.use_certificate_chain_file("cert.pem");
  ssl_context_.use_private_key_file("key.pem", asio::ssl::context::pem);
  ssl_context_.use_tmp_dh_file("dh4096.pem");
}

connection_ptr https_server::create_connection(asio::ip::tcp::socket socket, connection_manager& cm, request_handler& rh)
{
  return std::make_shared<ssl_connection>(std::move(socket), cm, rh, ssl_context_);
}

} // namespace f16::http::server
