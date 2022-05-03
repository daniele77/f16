// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_REQUEST_HPP
#define F16_HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include "header.hpp"

namespace f16 {
namespace http {
namespace server {

/// A request received from a client.
struct request
{
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  std::vector<header> headers;
};

} // namespace server
} // namespace http
} // namespace f16

#endif // F16_HTTP_REQUEST_HPP
