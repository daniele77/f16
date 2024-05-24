// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include "dynamic_content.hpp"
#include "mime_types.hpp"
#include "reply.hpp"

namespace f16::http::server {

dynamic_content_handler::dynamic_content_handler(std::string _action, std::vector<std::string> _params, std::function<void(const request&, std::ostream&)> _handler) : 
  action(std::move(_action)),
  handler(std::move(_handler)),
  param_keys(std::move(_params))
{
}

void dynamic_content_handler::serve(const std::string& request_path, const http_request& http_req, reply& rep)
{
  request req(http_req);

  // get parameters:
  const char delimiter = '/';
	std::string temp;
	std::stringstream stringstream { request_path };
	std::vector<std::string> components_vec;
	while (std::getline(stringstream, temp, delimiter)) {
		if (!temp.empty())
      components_vec.push_back(temp);
  }

  for (std::size_t i = 0; i < param_keys.size() && i < components_vec.size(); ++i)
    req.add_param(param_keys[i], components_vec[i]);

  // req.dump();

  rep.status = reply::ok;
  std::ostringstream ss;
  handler(req, ss);
  rep.content = ss.str();
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type("txt");
}

} // namespace f16::http::server
