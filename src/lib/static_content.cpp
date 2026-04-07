// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include "static_content.hpp"
#include "reply.hpp"
#include "mime_types.hpp"
#include "http_request.hpp"
#include "string.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace f16::http::server {

static_content::static_content(std::string _doc_root)
  : doc_root(std::move(_doc_root))
{
}

bool static_content::serve_if_match(const std::string& location, const std::string& request_path, const http_request& req, reply& rep) const
{
  auto res_query = split_string(request_path);
  const auto& resource = res_query.first;
  const auto& query = res_query.second;

  if (resource.rfind(location, 0) != 0) // does not starts with
    return false;

  fs::path resource_path{resource.substr(location.size())};
  resource_path = doc_root / resource_path.relative_path();

  if (fs::is_directory(resource_path))
  {
    if (!req.uri.empty() && req.uri.back() != '/')
    {
      // directory w/o trailing slash
      rep = reply::stock_reply(reply::moved_permanently);

      std::string redirect_target = resource + '/';
      if (!query.empty())
        redirect_target += '?' + query;

      rep.add_header("Location", redirect_target);
    }
    else
    {
      // try adding index.html
      const fs::path index_path = resource_path / "index.html";

      if (fs::exists(index_path))
        serve_file(index_path, rep);
      else
        list_directory(resource_path, rep);
    }
  }
  else
  {
    // Open the file to send back.
    serve_file(resource_path, rep);
  }

  if (req.method == "HEAD")
    rep.clear_content();

  return true;
}

void static_content::list_directory(const fs::path& full_path, reply& rep)
{
  try
  {
    std::ostringstream ss;

    ss <<
      "<!DOCTYPE html>\r\n"
      "<html lang=\"en\">\r\n"
      "<head>\r\n"
      "<meta charset=\"utf-8\">\r\n"
      "<title>Directory listing</title>\r\n"
      "</head>\r\n"
      "<body>\r\n"
      "<h1>Directory listing</h1>\r\n"
      "<hr>\r\n"
      "<ul>\r\n";
    for (const auto& entry : fs::directory_iterator(full_path))
    {
      if (!entry.is_regular_file() && !entry.is_directory()) continue;
      auto name = entry.path().filename().string();
      if (entry.is_directory())
        name += '/';
      ss << "<li><a href=\"" << name << "\">" << name << "</a></li>\r\n";
    }
    ss <<
      "</ul>\r\n"
      "<hr>\r\n"
      "</body>\r\n"
      "</html> \r\n";

    rep = reply(ss.str(), reply::ok, mime_types::extension_to_type(".html"));
  }
  catch (const std::exception&)
  {
    rep = reply::stock_reply(reply::not_found);
  }
}

void static_content::serve_file(const fs::path& full_path, reply& rep)
{
  if (!fs::exists(full_path))
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }
  if (!fs::is_regular_file(full_path))
  {
    rep = reply::stock_reply(reply::forbidden);
    return;
  }
  std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
  if (!is)
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }

  const auto extension = full_path.extension();

  // Fill out the reply to be sent to the client.
  std::string content;
  std::array<char, 512> buf; // NOLINT
  while (is.read(buf.data(), buf.size()).gcount() > 0)
    content.append(buf.data(), static_cast<long unsigned int>(is.gcount()));
  rep = reply(content, reply::ok, mime_types::extension_to_type(extension.string()));
}

} // namespace f16::http::server
