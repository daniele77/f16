// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_PATH_ROUTER_HPP
#define F16_HTTP_PATH_ROUTER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>

#include "static_content.hpp"
#include "dynamic_content.hpp"

namespace f16::http::server {

/// Find the handler to use for a given path.
class path_router
{
public:
  /// Add a resource to the server
  template<typename T>
  void add(std::string location, T&& resource)
  {
    auto& handlers = resources[resource.method()];
    handlers.emplace_back(std::move(location), std::forward<T>(resource));
    std::sort(handlers.begin(), handlers.end(), [](const resource_entry& a, const resource_entry& b) {
      return a.location.size() > b.location.size();
    });
  }

  void operator()(const http_request& req, reply& rep) const;

private:

  struct resource_entry
  {
    template <typename Handler>
    resource_entry(std::string l, Handler&& h) :
      location(std::move(l)),
      handler(std::in_place_type<std::decay_t<Handler>>, std::forward<Handler>(h))
    {
      static_assert(std::disjunction<
        std::is_same<std::decay_t<Handler>, static_content>,
        std::is_same<std::decay_t<Handler>, dynamic_content>>::value,
        "Invalid handler type passed to resource_entry");      
    }

    bool serve_if_match(const std::string& resource_path, const http_request& req, reply& rep) const
    {
      return std::visit(
        [&](auto&& _handler) {
          return _handler.serve_if_match(location, resource_path, req, rep);
        },
        handler);
    }

    std::string location;
  private:
    std::variant<static_content, dynamic_content> handler;
  };

  // method -> list of resource_entry sorted by location length (desc)
  std::unordered_map<std::string, std::vector<resource_entry>> resources;
};

} // namespace f16::http::server

#endif // F16_HTTP_PATH_ROUTER_HPP
