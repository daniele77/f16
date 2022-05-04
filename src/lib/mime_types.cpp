// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "mime_types.hpp"

namespace f16::http::server::mime_types {

struct mapping
{
  const char* extension;
  const char* mime_type;
} mappings[] =
{
  { "gif", "image/gif" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "jpg", "image/jpeg" },
  { "png", "image/png" }
};

std::string extension_to_type(const std::string& extension)
{
  for (mapping m: mappings)
  {
    if (m.extension == extension)
    {
      return m.mime_type;
    }
  }

  return "text/plain";
}

} // namespace f16::http::server::mime_types
