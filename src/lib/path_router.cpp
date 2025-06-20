// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "path_router.hpp"
#include "http_request.hpp"
#include "url.hpp"
#include "reply.hpp"
#include <algorithm>

namespace f16::http::server {

void path_router::add(const std::string& location, std::unique_ptr<http_handler> resource)
{
  auto& handlers = resources[resource->method()];

  handlers.emplace_back(location, std::move(resource));
  std::sort(handlers.begin(), handlers.end(), [](const resource_entry& a, const resource_entry& b) {
    return a.location.size() > b.location.size();
  });
}

void path_router::handle_request(const http_request& req, reply& rep) const
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

  // we don't have handlers for HEAD methods.
  // use GET instead
  auto method = req.method;
  if (method == "HEAD")
    method = "GET";

  auto it = resources.find(method);
  if (it == resources.end())
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }

  auto found = std::find_if(it->second.begin(), it->second.end(), [&](const resource_entry& r) {
    return request_path.rfind(r.location, 0) == 0;
  });

  if (found != it->second.end())
  {
    const std::string resource_path = request_path.substr(found->location.size());
    found->handler->serve(resource_path, req, rep);
    return;
  }
  rep = reply::stock_reply(reply::not_found);
}

} // namespace f16::http::server
