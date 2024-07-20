// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_STATIC_CONTENT_HPP
#define F16_HTTP_STATIC_CONTENT_HPP

#include <string>
#include <filesystem>
#include "http_handler.hpp"

namespace f16::http::server {

class static_content_handler : public http_handler
{
public:
  explicit static_content_handler(const std::string& _doc_root);
  void serve(const std::string& _request_path, const http_request& req, reply& rep) override;
  [[nodiscard]] std::string method() const override { return "GET"; }

private:
  static void list_directory(const std::filesystem::path& full_path, reply& rep);
  static void serve_file(const std::filesystem::path& full_path, reply& rep);
  const std::filesystem::path doc_root;
};

inline std::unique_ptr<static_content_handler> static_content(const std::string& _doc_root)
{
  return std::make_unique<static_content_handler>(_doc_root);
}

} // namespace f16::http::server

#endif // F16_HTTP_STATIC_CONTENT_HPP
