// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "plain_connection.hpp"
#include <utility>
#include <vector>
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace f16::http::server {

plain_connection::plain_connection(asio::ip::tcp::socket socket,
    connection_manager& manager, request_handler& handler)
  : base_connection(std::move(socket), manager, handler)
{
}

void plain_connection::start()
{
  do_read();
}

} // namespace f16::http::server
