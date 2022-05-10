// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_DYNAMIC_CONTENT_GET_HPP
#define F16_HTTP_DYNAMIC_CONTENT_GET_HPP

#include "http_handler.hpp"
#include <string>

namespace f16::http::server {

class dynamic_content_get : public http_handler
{
public:
  explicit dynamic_content_get(const std::function<void(std::ostream&)>& handler);
  void serve(const std::string& request_path, const request& req, reply& rep) override;

private:
  const std::function<void(std::ostream&)> handler;
};

} // namespace f16::http::server

#endif // F16_HTTP_DYNAMIC_CONTENT_GET_HPP