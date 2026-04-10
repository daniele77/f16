// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "request_parser.hpp"
#include "http_request.hpp"
#include <charconv>

namespace f16::http::server {

request_parser::request_parser()
  : state_(method_start),
    content_length_(0),
    body_bytes_remaining_(0)
{
}

void request_parser::reset()
{
  state_ = method_start;
  content_length_ = 0;
  body_bytes_remaining_ = 0;
}

#if 0
std::tuple<bool, std::size_t> request_parser::parse_content_length(const http_request& req) const
{
  std::size_t content_length = 0;
  bool has_content_length = false;

  for (const auto& h : req.headers)
  {
    std::string name = h.name;
    std::transform(name.begin(), name.end(), name.begin(),
      [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); });

    if (name != "content-length")
      continue;

    if (has_content_length)
      return std::make_tuple(false, 0);

    has_content_length = true;

    const auto* begin = h.value.data();
    const auto* end = begin + h.value.size();
    std::size_t parsed_value = 0;
    const auto result = std::from_chars(begin, end, parsed_value);
    if (result.ec != std::errc{} || result.ptr != end)
      return std::make_tuple(false, 0);

    content_length = parsed_value;
  }

  return std::make_tuple(true, content_length);
}
#endif

request_parser::result_type request_parser::consume(http_request& req, char input) // NOLINT
{
  switch (state_)
  {
  case method_start:
    if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return bad;
    }
    else
    {
      state_ = method;
      req.method.push_back(input);
      return indeterminate;
    }
  case method:
    if (input == ' ')
    {
      state_ = uri;
      return indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return bad;
    }
    else
    {
      req.method.push_back(input);
      return indeterminate;
    }
  case uri:
    if (input == ' ')
    {
      state_ = http_version_h;
      return indeterminate;
    }
    else if (is_ctl(input))
    {
      return bad;
    }
    else
    {
      req.uri.push_back(input);
      return indeterminate;
    }
  case http_version_h:
    if (input == 'H')
    {
      state_ = http_version_t_1;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_t_1:
    if (input == 'T')
    {
      state_ = http_version_t_2;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_t_2:
    if (input == 'T')
    {
      state_ = http_version_p;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_p:
    if (input == 'P')
    {
      state_ = http_version_slash;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_slash:
    if (input == '/')
    {
      req.http_version_major = 0;
      req.http_version_minor = 0;
      state_ = http_version_major_start;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_major_start:
    if (is_digit(input))
    {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      state_ = http_version_major;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_major:
    if (input == '.')
    {
      state_ = http_version_minor_start;
      return indeterminate;
    }
    else if (is_digit(input))
    {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_minor_start:
    if (is_digit(input))
    {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      state_ = http_version_minor;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case http_version_minor:
    if (input == '\r')
    {
      state_ = expecting_newline_1;
      return indeterminate;
    }
    else if (is_digit(input))
    {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case expecting_newline_1:
    if (input == '\n')
    {
      state_ = header_line_start;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case header_line_start:
    if (input == '\r')
    {
      state_ = expecting_newline_3;
      return indeterminate;
    }
    else if (!req.headers.empty() && (input == ' ' || input == '\t'))
    {
      state_ = header_lws;
      return indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return bad;
    }
    else
    {
      req.headers.emplace_back();
      req.headers.back().name.push_back(input);
      state_ = header_name;
      return indeterminate;
    }
  case header_lws:
    if (input == '\r')
    {
      state_ = expecting_newline_2;
      return indeterminate;
    }
    else if (input == ' ' || input == '\t')
    {
      return indeterminate;
    }
    else if (is_ctl(input))
    {
      return bad;
    }
    else
    {
      state_ = header_value;
      req.headers.back().value.push_back(input);
      return indeterminate;
    }
  case header_name:
    if (input == ':')
    {
      state_ = space_before_header_value;
      return indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return bad;
    }
    else
    {
      req.headers.back().name.push_back(input);
      return indeterminate;
    }
  case space_before_header_value:
    if (input == ' ')
    {
      state_ = header_value;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case header_value:
    if (input == '\r')
    {
      state_ = expecting_newline_2;
      
      std::string name = req.headers.back().name;
      std::transform(name.begin(), name.end(), name.begin(),
        [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); });

      if (name == "content-length")
      {
        const auto* begin = req.headers.back().value.data();
        const auto* end = begin + req.headers.back().value.size();
        const auto result = std::from_chars(begin, end, content_length_);
        if (result.ec != std::errc{} || result.ptr != end)
          return bad;
      }

      return indeterminate;
    }
    else if (is_ctl(input))
    {
      return bad;
    }
    else
    {
      req.headers.back().value.push_back(input);
      return indeterminate;
    }
  case expecting_newline_2:
    if (input == '\n')
    {
      state_ = header_line_start;
      return indeterminate;
    }
    else
    {
      return bad;
    }
  case expecting_newline_3:
    if (input != '\n')
      return bad;

    {
#if 0
      bool ok = true;
      std::size_t content_length = 0;
      std::tie(ok, content_length) = parse_content_length(req);
      if (!ok)
        return bad;

      if (content_length == 0)
        return good;

      body_bytes_remaining_ = content_length;
      req.body.clear();
      req.body.reserve(content_length);
      state_ = body;
      return indeterminate;
#else
      if (content_length_ == 0)
        return good;
      body_bytes_remaining_ = content_length_;
      req.body.clear();
      req.body.reserve(content_length_);
      state_ = body;
      return indeterminate;
#endif
    }
  case body:
    req.body.push_back(input);
    --body_bytes_remaining_;
    if (body_bytes_remaining_ == 0)
      return good;
    return indeterminate;
  default:
    return bad;
  }
}

constexpr bool request_parser::is_char(int c)
{
  return c >= 0 && c <= 127;
}

constexpr bool request_parser::is_ctl(int c)
{
  return (c >= 0 && c <= 31) || (c == 127);
}

constexpr bool request_parser::is_tspecial(int c)
{
  switch (c)
  {
  case '(': case ')': case '<': case '>': case '@':
  case ',': case ';': case ':': case '\\': case '"':
  case '/': case '[': case ']': case '?': case '=':
  case '{': case '}': case ' ': case '\t':
    return true;
  default:
    return false;
  }
}

constexpr bool request_parser::is_digit(int c)
{
  return c >= '0' && c <= '9';
}

} // namespace f16::http::server
