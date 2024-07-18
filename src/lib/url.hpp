// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_URL_HPP
#define F16_HTTP_URL_HPP

#include <string>

namespace f16::http::server {

/**
 * @brief Decodes a URL-encoded string.
 * 
 * This function takes a URL-encoded input string and decodes it into a plain text output string.
 * URL encoding replaces certain characters with a '%' followed by two hexadecimal digits.
 * This function handles such replacements and also converts '+' characters to spaces.
 * 
 * @param[in] in The URL-encoded input string.
 * @param[out] out The decoded output string.
 * @return true If the decoding is successful.
 * @return false If the decoding fails due to incorrect encoding.
 */
  bool url_decode(const std::string& in, std::string& out);

} // namespace f16::http::server

#endif // F16_HTTP_URL_HPP
