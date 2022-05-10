// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include "static_content.hpp"
#include "reply.hpp"
#include "mime_types.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace f16::http::server {

static_content::static_content(std::string _doc_root)
  : doc_root(std::move(_doc_root))
{}

void static_content::serve(const std::string& _request_path, const request& /* req */, reply& rep)
{
  const std::string request_path = '/' + _request_path;
  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/')
  {
    try
    {
      const std::string full_path = doc_root + request_path;

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
      for (const auto& entry : fs::directory_iterator(full_path))
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
    catch (const std::exception&)
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
  const std::string full_path = doc_root + request_path;

  if (!fs::is_regular_file(full_path))
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }
  std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
  if (!is)
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }

  // Fill out the reply to be sent to the client.
  rep.status = reply::ok;
  std::array<char, 512> buf; // NOLINT
  while (is.read(buf.data(), buf.size()).gcount() > 0)
    rep.content.append(buf.data(), static_cast<long unsigned int>(is.gcount()));
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type(extension);
}

} // namespace f16::http::server
