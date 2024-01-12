// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "path_router.hpp"
#include "request.hpp"
#include <algorithm>
#include <cassert>

namespace f16::http::server {

void path_router::add(const std::string& location, const std::shared_ptr<http_handler>& resource)
{
  auto& handlers = resources[resource->method()];
  
  handlers.emplace_back(location, resource);
  std::sort(handlers.begin(), handlers.end(), [](const resource_entry& a, const resource_entry& b) {
    return a.location.size() > b.location.size();
  });
}

bool path_router::serve(const std::string& request_path, const request& req, reply& rep) const
{
  // we don't have handlers for HEAD methods.
  // use use GET instead
  auto method = req.method;
  if (method == "HEAD")
    method = "GET";

  auto it = resources.find(method);
  if (it == resources.end())
    return false;

  auto found = std::find_if(it->second.begin(), it->second.end(), [&](const resource_entry& r){
    return request_path.rfind(r.location, 0) == 0;
  });

  if (found != it->second.end())
  {
    const std::string resource_path = request_path.substr(found->location.size());
    found->handler->serve(resource_path, req, rep);
    return true;
  }

  return false;
}

} // namespace f16::http::server
