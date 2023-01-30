// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "server.hpp"
#include <utility>

namespace f16::http::server {

server::server(asio::io_context& ioc)
  : io_context_(ioc),
    acceptor_(io_context_)
{
}

server::~server()
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

void server::listen(const std::string& port, const std::string& address)
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


void server::add(const std::string& path, const std::shared_ptr<http_handler>& handler)
{ 
  request_handler_.add(path, handler);
}

void server::do_accept()
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
          connection_manager_.start(std::make_shared<connection>(
              std::move(socket), connection_manager_, request_handler_));
        }

        do_accept();
      });
}

} // namespace f16::http::server
