// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_STATIC_CONTENT_HPP
#define F16_HTTP_STATIC_CONTENT_HPP

#include <string>
#include <filesystem>

namespace f16::http::server {

// Forward declarations
struct http_request;
struct reply;

class static_content
{
public:
  explicit static_content(std::string _doc_root);
  bool serve_if_match(const std::string& location, const std::string& _request_path, const http_request& req, reply& rep) const;
  [[nodiscard]] static std::string method() { return "GET"; }

private:
  static void list_directory(const std::filesystem::path& full_path, reply& rep);
  static void serve_file(const std::filesystem::path& full_path, reply& rep);
  std::filesystem::path doc_root;
};

} // namespace f16::http::server

#endif // F16_HTTP_STATIC_CONTENT_HPP
