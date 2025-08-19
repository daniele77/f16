// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "request_handler.hpp"
#include "http_request.hpp"

// #include <iostream> // TODO remove

namespace f16::http::server {

void request_handler::set(handler_fn handler)
{
  router = std::move(handler);
}

void request_handler::handle_request(const http_request& req, reply& rep) const
{
  router(req, rep);
}

} // namespace f16::http::server
