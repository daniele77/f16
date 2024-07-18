// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <cctype>
#include "url.hpp"

namespace f16::http::server {

  bool url_decode(const std::string& in, std::string& out)
  {
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i)
    {
      if (in[i] == '%')
      {
        if (i + 2 < in.size())
        {
          char hex[3] = { in[i + 1], in[i + 2], '\0' };
          if (std::isxdigit(hex[0]) && std::isxdigit(hex[1]))
          {
            int value;
            std::istringstream is(hex);
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

