// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "mime_types.hpp"
#include "reply.hpp"
#include "dynamic_content_get.hpp"
#include <sstream>

namespace f16::http::server {

dynamic_content_get::dynamic_content_get(const std::function<void(std::ostream&)>& _handler) : 
  handler(_handler)
{
}

void dynamic_content_get::serve(const std::string& /* request_path */, const request& /* req */, reply& rep)
{
  rep.status = reply::ok;
  std::ostringstream ss;
  handler(ss);
  rep.content = ss.str();
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type("txt");
}

} // namespace f16::http::server
