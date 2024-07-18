// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_REQUEST_HANDLER_HPP
#define F16_HTTP_REQUEST_HANDLER_HPP

#include <string>
#include <unordered_map>
#include <functional>

#include "path_router.hpp"

namespace f16::http::server {

struct reply;
struct http_request;

/// The common handler for all incoming requests.
class request_handler
{
public:

  request_handler(const request_handler&) = delete;
  request_handler& operator=(const request_handler&) = delete;

  request_handler() = default;

  void add(const std::string& path, const std::shared_ptr<http_handler>& handler);

  /// Handle a request and produce a reply.
  void handle_request(const http_request& req, reply& rep) const;

private:
  path_router router;
};

} // namespace f16::http::server

#endif // F16_HTTP_REQUEST_HANDLER_HPP
