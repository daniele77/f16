// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request_parser.hpp"
#include "request.hpp"

using namespace f16::http::server;

TEST_CASE("mime types are deduced by extension", "[mime_types]")
{
  using namespace f16::http::server::mime_types;
  REQUIRE(extension_to_type("gif") == "image/gif");
  REQUIRE(extension_to_type("htm") == "text/html" );
  REQUIRE(extension_to_type("html") == "text/html" );
  REQUIRE(extension_to_type("jpg") == "image/jpeg" );
  REQUIRE(extension_to_type("png") == "image/png");
  REQUIRE(extension_to_type("unk") == "text/plain");
}

static void CheckEqual(asio::const_buffer b, const std::string& s)
{
  const char* buff = static_cast<const char*>(b.data());
  REQUIRE(b.size() == s.size());
  CHECK(std::equal(buff, buff+b.size(), s.begin())); // NOLINT
}

TEST_CASE("reply converts to buffer", "[reply]")
{
  reply rep;
  rep.status = reply::ok;
  rep.content = "body";
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type("html");

  const auto buffers = rep.to_buffers();

  REQUIRE(buffers.size() == 11);
  
  CheckEqual(buffers[0], "HTTP/1.0 200 OK\r\n");

  CheckEqual(buffers[1], "Content-Length");
  CheckEqual(buffers[2], ": ");
  CheckEqual(buffers[3], "4");
  CheckEqual(buffers[4], "\r\n");
  CheckEqual(buffers[5], "Content-Type");
  CheckEqual(buffers[6], ": ");
  CheckEqual(buffers[7], "text/html");
  CheckEqual(buffers[8], "\r\n");

  CheckEqual(buffers[9], "\r\n");

  CheckEqual(buffers[10], "body");
}

TEST_CASE("parser works properly", "[request_parser]") // NOLINT
{
  // Request-Line = Method SP Request-URI SP HTTP-Version CRLF

  request_parser grammar;
  request req;
  const std::string input{"GET /hello.htm HTTP/1.1\r\nAccept-Language: en-us\r\n\r\n"};
  const auto result = grammar.parse(req, input.begin(), input.end());
  CHECK(std::get<0>(result) == request_parser::good);
  CHECK(std::get<1>(result) == input.end());
  CHECK(req.method == "GET");
  CHECK(req.uri == "/hello.htm");
  CHECK(req.http_version_major == 1);
  CHECK(req.http_version_minor == 1);
  REQUIRE(req.headers.size() == 1);
  CHECK(req.headers[0].name == "Accept-Language");
  CHECK(req.headers[0].value == "en-us");
}