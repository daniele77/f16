// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "https_server.hpp"
#include "ssl_connection.hpp"

namespace f16::http::server {

https_server::https_server(asio::io_context& ioc, const ssl_settings& ssl_s)
  : http_server{ioc}
  , ssl_context_{asio::ssl::context::tlsv13}
{
  ssl_context_.set_options(
    asio::ssl::context::default_workarounds |
    // asio::ssl::context::no_sslv2 |
    asio::ssl::context::single_dh_use
  );
  for (const auto& proto: ssl_s.protocols)
    ssl_context_.set_options(proto);
  if (!ssl_s.ciphers.empty())
    SSL_CTX_set_cipher_list(ssl_context_.native_handle(), ssl_s.ciphers.c_str());
  if (ssl_s.prefer_server_ciphers)
    SSL_CTX_set_options(ssl_context_.native_handle(), SSL_OP_CIPHER_SERVER_PREFERENCE);
  if (!ssl_s.password.empty()) // only to decrypt private key 
  {
    auto psw = ssl_s.password;
    ssl_context_.set_password_callback(
      [psw](std::size_t /*size*/, asio::ssl::context::password_purpose /*purpose*/) -> std::string { return psw; }
    );
  }
  ssl_context_.use_certificate_chain_file(ssl_s.certificate);
  ssl_context_.use_private_key_file(ssl_s.certificate_key, asio::ssl::context::pem);
  ssl_context_.use_tmp_dh_file(ssl_s.dhparam);
  if (!ssl_s.client_certificate.empty())
  {
    ssl_context_.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    ssl_context_.load_verify_file(ssl_s.client_certificate);
  }

  // cache in SSL sessions
  if (ssl_s.session_cache)
    SSL_CTX_set_session_cache_mode(ssl_context_.native_handle(), SSL_SESS_CACHE_SERVER);
    // Imposta la dimensione della cache delle sessioni SSL (in numero di sessioni)
  if (ssl_s.session_cache_size > 0)
    SSL_CTX_sess_set_cache_size(ssl_context_.native_handle(), ssl_s.session_cache_size);

  // Set timeout SSL sessions (in seconds)
  if (ssl_s.session_timeout_secs > 0)
    SSL_CTX_set_timeout(ssl_context_.native_handle(), ssl_s.session_timeout_secs);
}

connection_ptr https_server::create_connection(asio::ip::tcp::socket socket, connection_manager& cm, request_handler& rh)
{
  return std::make_shared<ssl_connection>(std::move(socket), cm, rh, ssl_context_);
}

} // namespace f16::http::server
