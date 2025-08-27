// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <string>
#include "dynamic_content.hpp"
#include "mime_types.hpp"
#include "reply.hpp"
#include "string.hpp"
#include "request.hpp"

namespace f16::http::server {

dynamic_content::dynamic_content(std::string _action, std::function<void(const request&, response_stream&)> _handler) : 
  action{std::move(_action)},
  handler{std::move(_handler)}
{
}

bool dynamic_content::serve_if_match(const std::string& location, const std::string& request_path, const http_request& http_req, reply& rep) const
{
  auto res_query = split_string(request_path);
  const auto& resources = res_query.first;
  const auto& query = res_query.second;

  request req{http_req};
  if (!match_pattern(location, resources, req.resources))
    return false;

  handle_query_parameters(query, req);

  response_stream ss;
  handler(req, ss);
  rep.content = ss.str();
  rep.status = ss.status;
  rep.headers = {
    {"Content-Length", std::to_string(rep.content.size())},
    // {"Content-Type", mime_types::extension_to_type(".txt")} // TODO: use a more appropriate content type
    {"Content-Type", ss.content_type}
  };

  return true;
}

void dynamic_content::handle_query_parameters(const std::string& query, request& req) {
  std::vector<std::string> params;
  std::stringstream s{query};
  std::string temp;

  while (std::getline(s, temp, '&')) {
    params.push_back(temp);
  }

  for (const auto& param : params) {
    auto key_value = split_string(param, '=');
    req.add_queryvalue(key_value.first, key_value.second);
  }
}

} // namespace f16::http::server
