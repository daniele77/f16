// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "https_server.hpp"
#include <utility>

namespace f16::http::server {

https_server::https_server(asio::io_context& ioc)
  : io_context_(ioc),
    ssl_context_(asio::ssl::context::tlsv13),
    acceptor_(io_context_)
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

https_server::~https_server()
{
  try
  {    
    acceptor_.close();
    connection_manager_.stop_all();
  }
  catch(...)
  {
    // nothing to do in the destructor
  }
}

void https_server::listen(const std::string& port, const std::string& address)
{
  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  asio::ip::tcp::resolver resolver(io_context_);
  const asio::ip::tcp::endpoint endpoint =
    *resolver.resolve(address, port).begin();
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  do_accept();
}

void https_server::add(const std::string& path, std::unique_ptr<http_handler> handler)
{ 
  request_handler_.add(path, std::move(handler));
}

void https_server::do_accept()
{
  acceptor_.async_accept(
    [this](std::error_code ec, asio::ip::tcp::socket socket)
    {
      // Check whether the server was stopped by a signal before this
      // completion handler had a chance to run.
      if (!acceptor_.is_open())
      {
        return;
      }

      if (!ec)
      {
        connection_manager_.start(std::make_shared<ssl_connection>(
            std::move(socket), connection_manager_, request_handler_, ssl_context_));
      }

      do_accept();
    });
}

} // namespace f16::http::server
