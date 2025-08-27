// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_DYNAMIC_CONTENT_HPP
#define F16_HTTP_DYNAMIC_CONTENT_HPP

#include <string>
#include <memory>
#include <functional>
#include <sstream>
#include "reply.hpp"

namespace f16 {
struct response_stream : public std::ostringstream {
  std::string content_type = "text/plain"; // default
  http::server::reply::status_type status = http::server::reply::ok; // default status
};

// f16 manipolators for response_stream
inline response_stream& json(response_stream& os) {
  os.content_type = "application/json";
  return os;
}

inline response_stream& xml(response_stream& os) {
  os.content_type = "application/xml";
  return os;
}

inline response_stream& plain(response_stream& os) {
  os.content_type = "text/plain";
  return os;
}

inline response_stream& ok(response_stream& os) {
  os.status = http::server::reply::ok;
  return os;
}

inline response_stream& bad_request(response_stream& os) {
  os.status = http::server::reply::bad_request;
  return os;
}

inline response_stream& operator<<(response_stream& os, response_stream& (*f)(response_stream&)) {
  return f(os);
}

} // namespace f16

namespace f16::http::server {

// forward declarations
struct http_request;
struct request;
struct reply;

class dynamic_content
{
public:
  dynamic_content(std::string action, std::function<void(const request& req, response_stream&)> _handler);
  bool serve_if_match(const std::string& location, const std::string& request_path, const http_request& req, reply& rep) const;
  [[nodiscard]] std::string method() const { return action; }

private:
  static void handle_query_parameters(const std::string& query, request& req);

  std::string action;
  std::function<void(const request&, response_stream&)> handler;
};

inline dynamic_content get(std::function<void(const request& req, response_stream&)> _handler)
{
  return dynamic_content("GET", _handler);
}

inline dynamic_content post(std::function<void(const request& req, response_stream&)> _handler)
{
  return dynamic_content("POST", _handler);
}

inline dynamic_content put(std::function<void(const request& req, response_stream&)> _handler)
{
  return dynamic_content("PUT", _handler);
}

} // namespace f16::http::server

#endif // F16_HTTP_DYNAMIC_CONTENT_HPP
