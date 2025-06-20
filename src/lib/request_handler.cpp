// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "request_handler.hpp"
#include "http_request.hpp"

// #include <iostream> // TODO remove

namespace f16::http::server {

#ifndef NEW_CODE
void request_handler::set(path_router handler)
{
  router = std::move(handler);
}
#endif

void request_handler::set(handler_fn handler)
{
  router = std::move(handler);
}

void request_handler::handle_request(const http_request& req, reply& rep) const
{
#ifndef NEW_CODE
  // std::visit([req, &rep](const auto& r){ r.handle_request(req, rep); }, router);
  std::visit([req, &rep](const auto& r){ r(req, rep); }, router);
#else
  router(req, rep);
#endif
}

} // namespace f16::http::server
