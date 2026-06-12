// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "request_handler.hpp"
#include "f16/http_request.hpp"
#include <utility>

namespace f16::http::server
{

void request_handler::set(handler_fn handler)
{
  router = std::move(handler);
}

void request_handler::handle_request(const http_request& req, reply& rep) const
{
  router(req, rep);
}

} // namespace f16::http::server
