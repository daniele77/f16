// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_DYNAMIC_CONTENT_HPP
#define F16_HTTP_DYNAMIC_CONTENT_HPP

#include <string>
#include <memory>
#include <functional>
#include "http_handler.hpp"
#include "request.hpp"

namespace f16::http::server {

class dynamic_content_handler : public http_handler
{
public:
  explicit dynamic_content_handler(std::string action, std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler);
  void serve(const std::string& request_path, const http_request& req, reply& rep) override;
  [[nodiscard]] std::string method() const override { return action; }

private:
  const std::string action;
  const std::function<void(const request&, std::ostream&)> handler;
  const std::vector<std::string> param_keys;
};

inline std::shared_ptr<dynamic_content_handler> get(std::function<void(const request& req, std::ostream&)> _handler)
{
  return std::make_shared<dynamic_content_handler>("GET", std::vector<std::string>{}, _handler);
}

inline std::shared_ptr<dynamic_content_handler> get(std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler)
{
  return std::make_shared<dynamic_content_handler>("GET", std::move(params), _handler);
}

inline std::shared_ptr<dynamic_content_handler> post(std::function<void(const request& req, std::ostream&)> _handler)
{
  return std::make_shared<dynamic_content_handler>("POST", std::vector<std::string>{}, _handler);
}

inline std::shared_ptr<dynamic_content_handler> post(std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler)
{
  return std::make_shared<dynamic_content_handler>("POST", std::move(params), _handler);
}

inline std::shared_ptr<dynamic_content_handler> put(std::function<void(const request& req, std::ostream&)> _handler)
{
  return std::make_shared<dynamic_content_handler>("PUT", std::vector<std::string>{}, _handler);
}

inline std::shared_ptr<dynamic_content_handler> put(std::vector<std::string> params, std::function<void(const request& req, std::ostream&)> _handler)
{
  return std::make_shared<dynamic_content_handler>("PUT", std::move(params), _handler);
}

} // namespace f16::http::server

#endif // F16_HTTP_DYNAMIC_CONTENT_HPP
