// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "path_router.hpp"
#include <algorithm>
#include <cassert>

namespace f16::http::server {

void path_router::add(const std::string& location, std::shared_ptr<http_handler> resource)
{
  resources.emplace_back(location, resource);
  std::sort(resources.begin(), resources.end(), [](const resource_entry& a, const resource_entry& b) {
    return a.location.size() > b.location.size();
  });
}

bool path_router::serve(const std::string& request_path, const request& req, reply& rep) const
{
  for (const resource_entry& r : resources)
    if (request_path.rfind(r.location, 0) == 0)
    {
      //assert(r.location.size() >= 1); // NOLINT
      //const std::string resource_path = request_path.substr(r.location.size()-1);
      const std::string resource_path = request_path.substr(r.location.size());
      r.handler->serve(resource_path, req, rep);
      return true;
    }

  return false;
}

} // namespace f16::http::server
