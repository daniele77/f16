// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <unordered_map>
#include "mime_types.hpp"

namespace f16::http::server::mime_types {

static const std::unordered_map<std::string, std::string> mappings = // NOLINT
{
  { ".gif", "image/gif" },
  { ".htm", "text/html" },
  { ".html", "text/html" },
  { ".jpg", "image/jpeg" },
  { ".png", "image/png" },
  { ".txt", "text/plain" },
  { ".css", "text/css" },
  { ".js", "application/javascript" },
  { ".json", "application/json" },
  { ".xml", "application/xml" },
  { ".pdf", "application/pdf" },
  { ".zip", "application/zip" },
  { ".tar", "application/x-tar" },
  { ".gz", "application/gzip" },
  { ".mp3", "audio/mpeg" },
  { ".mp4", "video/mp4" },
  { ".webm", "video/webm" }
  // Add more mappings as needed
};

std::string extension_to_type(const std::string& extension)
{
  auto it = mappings.find(extension);
  if (it != mappings.end())
    return it->second;

  return "text/plain";
}

} // namespace f16::http::server::mime_types
