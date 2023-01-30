// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include "static_content.hpp"
#include "reply.hpp"
#include "mime_types.hpp"
#include "request.hpp"
#include <fstream>

namespace fs = std::filesystem;

namespace f16::http::server {

static_content::static_content(const std::string& _doc_root)
  : doc_root(_doc_root)
{
}

void static_content::serve(const std::string& _request_path, const request& req, reply& rep)
{
  fs::path request_path{_request_path};
  request_path = doc_root / request_path.relative_path();

  if (fs::is_directory(request_path))
  {
    // try adding index.html
    const fs::path index_path = request_path / "index.html";

    if (fs::exists(index_path))
      serve_file(index_path, rep);
    else if (!req.uri.empty() && req.uri.back() == '/')
      list_directory(request_path, rep);
    else
    {
      // directory w/o trailing slash
      rep = reply::stock_reply(reply::moved_permanently);
      const header h{"Location", req.uri + '/'};
      rep.headers.push_back(h);
    }
  }
  else
  {
    // Open the file to send back.
    serve_file(request_path, rep);
  }

  if (req.method == "HEAD")
    rep.content.clear();
}

void static_content::list_directory(const fs::path& full_path, reply& rep)
{
  try
  {
    rep.status = reply::ok;
    std::ostringstream ss;

    ss << 
      "<!DOCTYPE html>\r\n"
      "<html>\r\n"
      "<head><title>Directory listing</title></head>\r\n"
      "<body>\r\n";
    for (const auto& entry : fs::directory_iterator(full_path))
    {
      if (!entry.is_regular_file()) continue;
      const auto name = entry.path().filename().string();
      ss << "<a href=\"" << name << "\">" << name << "</a><br>\r\n";
    }
    ss <<
      "</body>\r\n"
      "</html> \r\n";

    rep.content = ss.str();
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = mime_types::extension_to_type(".html");
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
  rep.status = reply::ok;
  std::array<char, 512> buf; // NOLINT
  while (is.read(buf.data(), buf.size()).gcount() > 0)
    rep.content.append(buf.data(), static_cast<long unsigned int>(is.gcount()));
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type(extension.string());
}

} // namespace f16::http::server
