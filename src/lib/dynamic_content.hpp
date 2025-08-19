// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_DYNAMIC_CONTENT_HPP
#define F16_HTTP_DYNAMIC_CONTENT_HPP

#include <string>
#include <memory>
#include <functional>

namespace f16::http::server {

// forward declarations
struct http_request;
struct request;
struct reply;

class dynamic_content
{
public:
  dynamic_content(std::string action, std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler);
  void serve(const std::string& request_path, const http_request& req, reply& rep) const;
  [[nodiscard]] std::string method() const { return action; }

private:
  void get_path_components(const std::string& resources, request& req) const;
  static void handle_query_parameters(const std::string& query, request& req);

  std::string action;
  std::function<void(const request&, std::ostream&)> handler;
  std::vector<std::string> param_keys;
};

inline dynamic_content get(std::function<void(const request& req, std::ostream&)> _handler)
{
  return dynamic_content("GET", std::vector<std::string>{}, _handler);
}

inline dynamic_content get(std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler)
{
  return dynamic_content("GET", std::move(params), _handler);
}

inline dynamic_content post(std::function<void(const request& req, std::ostream&)> _handler)
{
  return dynamic_content("POST", std::vector<std::string>{}, _handler);
}

inline dynamic_content post(std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler)
{
  return dynamic_content("POST", std::move(params), _handler);
}

inline dynamic_content put(std::function<void(const request& req, std::ostream&)> _handler)
{
  return dynamic_content("PUT", std::vector<std::string>{}, _handler);
}

inline dynamic_content put(std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler)
{
  return dynamic_content("PUT", std::move(params), _handler);
}

} // namespace f16::http::server

#endif // F16_HTTP_DYNAMIC_CONTENT_HPP
