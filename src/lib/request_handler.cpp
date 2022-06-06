// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "request_handler.hpp"
#include <string>
#include "reply.hpp"
#include "request.hpp"

#include <iostream> // TODO remove

namespace f16::http::server {

void request_handler::add(const std::string& path, const std::shared_ptr<http_handler>& handler)
{ 
  router.add(path, handler);
}

void request_handler::handle_request(const request& req, reply& rep) const
{
  std::cout << "http v. " << req.http_version_major << '.' << req.http_version_minor
            << ' ' << req.method
            << " " << req.uri << std::endl; // TODO remove
  // Decode url to path.
  std::string request_path;
  if (!url_decode(req.uri, request_path))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos)
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  if (!router.serve(request_path, req, rep))
    rep = reply::stock_reply(reply::not_found);
}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (in[i] == '+')
    {
      out += ' ';
    }
    else
    {
      out += in[i];
    }
  }
  return true;
}

} // namespace f16::http::server
