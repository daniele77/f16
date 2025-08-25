// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_HTTP_REQUEST_HPP
#define F16_HTTP_HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <algorithm>
#include "header.hpp"

namespace f16::http::server {

/// A request received from a client.
struct http_request
{
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  std::vector<header> headers;

  std::string get_header(const std::string& name) const
  {
    auto it = std::find_if(headers.begin(), headers.end(),
      [&name](const header& h)
      {
        std::string header_name = h.name;
        std::transform(header_name.begin(), header_name.end(), header_name.begin(),
                [](char c){ return std::tolower(c); });
        return header_name == name;
      }
    );
    if (it != headers.end())
      return it->value;
    return {};
  }
};

} // namespace f16::http::server

#endif // F16_HTTP_HTTP_REQUEST_HPP
