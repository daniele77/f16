// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_PATH_ROUTER_HPP
#define F16_HTTP_PATH_ROUTER_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include <memory> // shared_ptr

#include "http_handler.hpp"

namespace f16::http::server {

/// Find the handler to use for a given path.
class path_router
{
public:
  /// Add a resource to the server
  void add(const std::string& location, const std::shared_ptr<http_handler>& resource);

  bool serve(const std::string& request_path, const request& req, reply& rep) const;

private:

  struct resource_entry
  {
    resource_entry(const std::string& l, std::shared_ptr<http_handler> h) : location(l), handler(h) {}
    std::string location;
    std::shared_ptr<http_handler> handler;
  };

  std::unordered_map<std::string, std::vector<resource_entry>> resources;
};

} // namespace f16::http::server

#endif // F16_HTTP_PATH_ROUTER_HPP
