// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_STRING_HPP
#define F16_HTTP_STRING_HPP

#include <string>
#include <unordered_map>
#include <utility>

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

  /**
   * @brief Matches an input string against a pattern and extracts parameters.
   *
   * This function checks if the given input string matches the specified pattern.
   * The pattern can include named parameters, denoted by a colon (e.g., ":param"),
   * which will be extracted and stored in the provided `params` map.
   *
   * @param pattern The pattern string to match against. Named parameters are prefixed with ':'.
   *                For example: "/user/:id/profile".
   * @param input The input string to be matched. For example: "/user/123/profile".
   * @param params A reference to an unordered map where extracted parameter names and values
   *               will be stored. The map is cleared at the beginning of the function.
   *               For example, given the pattern "/user/:id/profile" and input "/user/123/profile",
   *               the map will contain {"id": "123"}.
   * 
   * @return `true` if the input string matches the pattern, `false` otherwise.
   *
   * @note The function assumes that both the pattern and input strings use '/' as a separator.
   *       Trailing slashes in the input are allowed but optional.
   */
  inline bool match_pattern(const std::string& pattern, const std::string& input, std::unordered_map<std::string, std::string>& params)
  {
    params.clear();

    size_t pattern_pos = 0, input_pos = 0;

    while (pattern_pos < pattern.size() && input_pos < input.size())
    {
      if (pattern[pattern_pos] == ':')
      {
        auto param_name_end = pattern.find('/', pattern_pos + 1);
        auto param_name = pattern.substr(pattern_pos + 1, param_name_end - pattern_pos - 1);

        auto param_value_end = input.find('/', input_pos);
        params[param_name] = input.substr(input_pos, param_value_end - input_pos);

        pattern_pos = (param_name_end == std::string::npos) ? pattern.size() : param_name_end;
        input_pos = (param_value_end == std::string::npos) ? input.size() : param_value_end;
      }
      else if (pattern[pattern_pos++] != input[input_pos++])
      {
        return false;
      }
    }

    return pattern_pos == pattern.size() && (input_pos == input.size() || (input_pos + 1 == input.size() && input.back() == '/'));
  }

} // namespace f16::http::server

#endif // F16_HTTP_STRING_HPP
