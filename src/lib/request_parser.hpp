// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_REQUEST_PARSER_HPP
#define F16_HTTP_REQUEST_PARSER_HPP

#include <tuple>
#include <cstddef> // for std::size_t
#include <optional>

namespace f16::http::server {

struct http_request;

/// Parser for incoming requests.
class request_parser
{
public:
  struct limits
  {
    std::size_t max_request_line_bytes;
    std::size_t max_header_section_bytes;
    std::size_t max_headers_count;
    std::size_t max_body_bytes;

    limits()
      : max_request_line_bytes(8192)
      , max_header_section_bytes(32768)
      , max_headers_count(100)
      , max_body_bytes(1048576)
    {
    }

    limits(std::size_t request_line_bytes,
      std::size_t header_section_bytes,
      std::size_t headers_count,
      std::size_t body_bytes)
      : max_request_line_bytes(request_line_bytes)
      , max_header_section_bytes(header_section_bytes)
      , max_headers_count(headers_count)
      , max_body_bytes(body_bytes)
    {
    }
  };

  /// Construct ready to parse the request method.
  request_parser();
  explicit request_parser(limits parser_limits);

  /// Reset to initial parser state.
  // void reset();

  /// Result of parse.
  enum result_type { good, bad, too_large, indeterminate };

  /// Parse some data. The enum return value is good when a complete request has
  /// been parsed, bad if the data is invalid, indeterminate when more data is
  /// required. The InputIterator return value indicates how much of the input
  /// has been consumed.
  template <typename InputIterator>
  std::tuple<result_type, InputIterator> parse(http_request& req,
      InputIterator begin, InputIterator end)
  {
    while (begin != end)
    {
      result_type result = consume(req, *begin++);
      if (result == good || result == bad || result == too_large)
        return std::make_tuple(result, begin);
    }
    return std::make_tuple(indeterminate, begin);
  }

  bool is_reading_body() const { return state_ == body; }

private:
  /// Handle the next character of input.
  result_type consume(http_request& req, char input);

  /// Check if a byte is an HTTP character.
  static constexpr bool is_char(int c);

  /// Check if a byte is an HTTP control character.
  static constexpr bool is_ctl(int c);

  /// Check if a byte is defined as an HTTP tspecial character.
  static constexpr bool is_tspecial(int c);

  /// Check if a byte is a digit.
  static constexpr bool is_digit(int c);

  /// The current state of the parser.
  enum state
  {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3,
    body
  } state_;

  limits parser_limits_;
  std::optional<std::size_t> content_length_;
  std::size_t body_bytes_remaining_;
  std::size_t request_line_bytes_;
  std::size_t header_section_bytes_;

  bool count_request_line_byte();
  bool count_header_section_byte();
};

} // namespace f16::http::server

#endif // F16_HTTP_REQUEST_PARSER_HPP
