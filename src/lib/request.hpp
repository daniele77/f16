// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_REQUEST_HPP
#define F16_HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "http_request.hpp"
// #include <iostream> // TODO remove

namespace f16::http::server {

/// @brief  A request for the handler
struct request
{
  const http_request orig_request;
  std::unordered_map<std::string, std::string> params; // key -> value

  explicit request(const http_request& r) : orig_request(r) {}
  void add_param(const std::string& key, const std::string& value) { params[key] = value; }
  std::string param(const std::string& key) const { return params.at(key); }

/*
  void dump() const
  {
    std::cout << "method: " << orig_request.method << std::endl;
    std::cout << "uri: " << orig_request.uri << std::endl;
    std::cout << "version: " << orig_request.http_version_major << '.' << orig_request.http_version_minor << std::endl;
    for (const auto& h: orig_request.headers)
      std::cout << "header " << h.name << ": " << h.value << std::endl;
    for (const auto& p: params)
      std::cout << "parameter " << p.first << ": " << p.second << std::endl;
  }
*/
};

} // namespace f16::http::server

#endif // F16_HTTP_REQUEST_HPP
