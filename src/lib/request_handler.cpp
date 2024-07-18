// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "request_handler.hpp"
#include "reply.hpp"
#include "http_request.hpp"
#include "url.hpp"

// #include <iostream> // TODO remove

namespace f16::http::server {

void request_handler::add(const std::string& path, const std::shared_ptr<http_handler>& handler)
{ 
  router.add(path, handler);
}

void request_handler::handle_request(const http_request& req, reply& rep) const
{
  /*
  std::cout << "http v. " << req.http_version_major << '.' << req.http_version_minor
            << ' ' << req.method
            << " " << req.uri << std::endl; // TODO remove
  */
  // Decode url to path.  
  std::string request_path;
  if (!url_decode(req.uri, request_path))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos)
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  if (!router.serve(request_path, req, rep))
    rep = reply::stock_reply(reply::not_found);
}

} // namespace f16::http::server
