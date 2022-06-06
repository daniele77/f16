// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_HTTP_HANDLER_HPP
#define F16_HTTP_HTTP_HANDLER_HPP

#include <string>

namespace f16::http::server {

// forward declarations:
struct request;
struct reply;

class http_handler
{
public:
  virtual ~http_handler() = default;
  virtual void serve(const std::string& request_path, const request& req, reply& rep) = 0;
  virtual std::string method() const = 0;
};

} // namespace f16::http::server

#endif // F16_HTTP_HTTP_HANDLER_HPP
