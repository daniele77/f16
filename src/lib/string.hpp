// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_STRING_HPP
#define F16_HTTP_STRING_HPP

#include <string>

namespace f16::http::server {

  /**
   * @brief Splits a string into two parts based on a delimiter.
   *
   * This function takes an input string and a delimiter character, and splits the string
   * into two parts: the substring before the first occurrence of the delimiter (head) and 
   * the substring after the delimiter (tail). If the delimiter is not found, the entire 
   * input string is returned as the head and the tail is an empty string.
   *
   * @param[in] input The input string to be split.
   * @param[in] delim The delimiter character (default is '?').
   * @return A pair of strings, where the first element is the head and the second element is the tail.
   */
  inline std::pair<std::string, std::string> split_string(const std::string& input, char delim='?')
  {
    std::size_t pos = input.find(delim);
    if (pos != std::string::npos)
      return {input.substr(0, pos), input.substr(pos + 1)};
    else
      return {input, ""};
  }

} // namespace f16::http::server

#endif // F16_HTTP_STRING_HPP
