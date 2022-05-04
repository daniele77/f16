// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"

#include <filesystem>
namespace fs = std::filesystem;

namespace f16::http::server {

request_handler::request_handler(std::string doc_root)
  : doc_root_(std::move(doc_root))
{
}

void request_handler::handle_request(const request& req, reply& rep)
{
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

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/')
  {
    try
    {
        std::string full_path = doc_root_ + request_path;

        rep.status = reply::ok;
        std::ostringstream ss;

        ss << R"html(
              <!DOCTYPE html>
              <html>
              <head>
              <title>Directory list</title>
              </hed>
              <body>
              )html";
        for (const auto & entry : fs::directory_iterator(full_path))
        {
          if (!entry.is_regular_file()) continue;
          const auto name = entry.path().filename().string();
          ss << "<a href=\"" << name << "\">" << name << "</a><br>";
        }
        ss << R"html(
            <!DOCTYPE html>
            </body>
            </html> 
            )html";
        rep.content = ss.str();
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = mime_types::extension_to_type("html");
    }
    catch(const std::exception&)
    {
      rep = reply::stock_reply(reply::not_found);
    }

    return;
  }

  // Determine the file extension.
  std::size_t last_slash_pos = request_path.find_last_of('/');
  std::size_t last_dot_pos = request_path.find_last_of('.');
  std::string extension;
  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
  {
    extension = request_path.substr(last_dot_pos + 1);
  }

  // Open the file to send back.
  std::string full_path = doc_root_ + request_path;
  if(!fs::is_regular_file(full_path))
  {
    auto i = handlers.find(request_path);
    if (i != handlers.end())
    {
      rep.status = reply::ok;
      std::ostringstream ss;
      i->second(ss);
      rep.content = ss.str();
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = mime_types::extension_to_type("txt");
      return;
    }
    else
    {
      rep = reply::stock_reply(reply::not_found);
      return;
    }
  }
  std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
  if (!is)
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }

  // Fill out the reply to be sent to the client.
  rep.status = reply::ok;
  char buf[512];
  while (is.read(buf, sizeof(buf)).gcount() > 0) // NOLINT
    rep.content.append(buf, static_cast<long unsigned int>(is.gcount())); // NOLINT
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type(extension);
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

void request_handler::add_handler(const std::string& url, const std::function<void(std::ostream&)>& h)
{
  handlers[url] = h;
}

} // namespace f16::http::server
