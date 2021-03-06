// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_HEADER_HPP
#define F16_HTTP_HEADER_HPP

#include <string>

namespace f16::http::server {

struct header
{
  std::string name;
  std::string value;
};

} // namespace f16::http::server

#endif // F16_HTTP_HEADER_HPP
