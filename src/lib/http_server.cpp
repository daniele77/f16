// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "http_server.hpp"
#include "plain_connection.hpp"
#include <utility>

namespace f16::http::server {

http_server::http_server(asio::io_context& ioc)
  : io_context_(ioc),
    acceptor_(io_context_)
{
}

http_server::~http_server()
{
  try
  {    
    acceptor_.close();
  }
  catch(...)
  {
    // nothing to do in the destructor
  }
}

void http_server::listen(const std::string& port, const std::string& address)
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

void http_server::do_accept()
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
        connection_manager_.start(create_connection(
            std::move(socket), connection_manager_, request_handler_));
      }

      do_accept();
    });
}

connection_ptr http_server::create_connection(asio::ip::tcp::socket socket, connection_manager& cm, request_handler& rh)
{
  return std::make_shared<plain_connection>(std::move(socket), cm, rh);
}

} // namespace f16::http::server
