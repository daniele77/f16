// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "dynamic_content.hpp"
#include "http_request.hpp"
#include "mime_types.hpp"
#include "path_router.hpp"
#include "reply.hpp"
#include "request_parser.hpp"
#include "string.hpp"
#include "url.hpp"
#include "request.hpp"
#include <catch2/catch.hpp>

using namespace f16::http::server;

TEST_CASE("split_string splits strings correctly", "[split_string]")
{
  SECTION("Splits a string with the delimiter present")
  {
    auto result = split_string("hello?world");
    REQUIRE(result.first == "hello");
    REQUIRE(result.second == "world");
  }

  SECTION("Returns the entire string as head if the delimiter is not present")
  {
    auto result = split_string("helloworld");
    REQUIRE(result.first == "helloworld");
    REQUIRE(result.second == "");
  }

  SECTION("Splits a string with multiple occurrences of the delimiter")
  {
    auto result = split_string("hello?world?again");
    REQUIRE(result.first == "hello");
    REQUIRE(result.second == "world?again");
  }

  SECTION("Handles empty input string")
  {
    auto result = split_string("");
    REQUIRE(result.first == "");
    REQUIRE(result.second == "");
  }

  SECTION("Handles input string with delimiter at the start")
  {
    auto result = split_string("?helloworld");
    REQUIRE(result.first == "");
    REQUIRE(result.second == "helloworld");
  }

  SECTION("Handles input string with delimiter at the end")
  {
    auto result = split_string("helloworld?");
    REQUIRE(result.first == "helloworld");
    REQUIRE(result.second == "");
  }

  SECTION("Handles different delimiter")
  {
    auto result = split_string("hello/world", '/');
    REQUIRE(result.first == "hello");
    REQUIRE(result.second == "world");
  }
}

TEST_CASE("match_pattern matches an input string against a pattern correctly", "[match_pattern]")
{
  std::unordered_map<std::string, std::string> params;

  SECTION("Exact match without parameters")
  {
    REQUIRE(match_pattern("/user/profile", "/user/profile", params));
    REQUIRE(params.empty());
  }

  SECTION("Match with one parameter")
  {
    REQUIRE(match_pattern("/user/:id/profile", "/user/123/profile", params));
    REQUIRE(params.size() == 1);
    REQUIRE(params["id"] == "123");
  }

  SECTION("Match with multiple parameters")
  {
    REQUIRE(match_pattern("/user/:id/post/:postId", "/user/123/post/456", params));
    REQUIRE(params.size() == 2);
    REQUIRE(params["id"] == "123");
    REQUIRE(params["postId"] == "456");
  }

  SECTION("No match due to different structure")
  {
    REQUIRE_FALSE(match_pattern("/user/:id/profile", "/user/123", params));
  }

  SECTION("Trailing slash in input")
  {
    REQUIRE(match_pattern("/user/:id/profile", "/user/123/profile/", params));
    REQUIRE(params.size() == 1);
    REQUIRE(params["id"] == "123");
  }

  SECTION("No match due to different characters")
  {
    REQUIRE_FALSE(match_pattern("/user/:id/profile", "/user/abc/settings", params));
  }

  SECTION("Empty pattern")
  {
    REQUIRE_FALSE(match_pattern("", "/user/123/profile", params));
  }

  SECTION("Empty input")
  {
    REQUIRE_FALSE(match_pattern("/user/:id/profile", "", params));
  }
}

TEST_CASE("url_decode correctly decodes URL-encoded strings", "[url_decode]")
{
  std::string output;

  SECTION("Decodes a simple string with no encoded characters")
  {
    REQUIRE(url_decode("HelloWorld", output));
    REQUIRE(output == "HelloWorld");
  }

  SECTION("Decodes a string with spaces represented by '+'")
  {
    REQUIRE(url_decode("Hello+World", output));
    REQUIRE(output == "Hello World");
  }

  SECTION("Decodes a string with % encoded characters")
  {
    REQUIRE(url_decode("Hello%20World", output));
    REQUIRE(output == "Hello World");
  }

  SECTION("Decodes a string with multiple % encoded characters")
  {
    REQUIRE(url_decode("%48%65%6C%6C%6F%20%57%6F%72%6C%64", output));
    REQUIRE(output == "Hello World");
  }

  SECTION("Returns false for a string with incomplete % encoding") // NOLINT
  {
    REQUIRE_FALSE(url_decode("Hello%2World", output));
  }

  SECTION("Returns false for a string with invalid hex digits") // NOLINT
  {
    REQUIRE_FALSE(url_decode("Hello%2GWorld", output));
  }

  SECTION("Handles empty input string")
  {
    REQUIRE(url_decode("", output));
    REQUIRE(output.empty());
  }

  SECTION("Handles input string ending with '%'") // NOLINT
  {
    REQUIRE_FALSE(url_decode("HelloWorld%", output));
  }

  SECTION("Handles input string ending with '%2'") // NOLINT
  {
    REQUIRE_FALSE(url_decode("HelloWorld%2", output));
  }
}

TEST_CASE("mime types are deduced by extension", "[mime_types]")
{
  using namespace f16::http::server::mime_types;
  REQUIRE(extension_to_type(".gif") == "image/gif");
  REQUIRE(extension_to_type(".htm") == "text/html");
  REQUIRE(extension_to_type(".html") == "text/html");
  REQUIRE(extension_to_type(".jpg") == "image/jpeg");
  REQUIRE(extension_to_type(".png") == "image/png");
  REQUIRE(extension_to_type(".unk") == "text/plain");
}

static void CheckEqual(asio::const_buffer b, const std::string& s)
{
  const char* buff = static_cast<const char*>(b.data());
  REQUIRE(b.size() == s.size());
  CHECK(std::equal(buff, buff + b.size(), s.begin())); // NOLINT
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
  rep.headers[1].value = mime_types::extension_to_type(".html");

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
  http_request req;
  const std::string input{ "GET /hello.htm HTTP/1.1\r\nAccept-Language: en-us\r\n\r\n" };
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

TEST_CASE("path_router routes simple requests", "[path_router]") // NOLINT
{
  std::vector<std::pair<int, std::string>> calls;

  auto make_handler = [&calls](int id) {
    return [id, &calls](const request& req, std::ostream& /*os*/) {
      calls.emplace_back(id, req.resource("1")+req.resource("2"));
    };
  };

  path_router router;
  router.add("/foo/:1", get(make_handler(1)));
  router.add("/foo/bar/:1", get(make_handler(2)));
  router.add("/bar/:1/:2", get(make_handler(3)));
  router.add("/bar/foo/aaa/:1", get(make_handler(4)));
  router.add("/bar/foo/bbb/:1", get(make_handler(5)));
  router.add("/bar/foo/bbb/ccc/:1", get(make_handler(6)));
  router.add("/foo/bar/aaa/:1", get(make_handler(7)));
  
  auto test_request = [&](const std::string& uri, reply::status_type expected_status, const std::string& method = "GET") {
    CAPTURE(method, uri, expected_status);
    http_request req;
    reply rep;
    req.uri = uri;
    req.method = method;
    router(req, rep);
    CHECK(rep.status == expected_status);
  };

  test_request("/ddd", reply::not_found);
  test_request("/bar/foo/xxx", reply::ok);
  test_request("/bar/foo/aaa/zzz", reply::ok);
  test_request("/bar/foo/bbb/xxx", reply::ok);
  test_request("/bar/foo/bbb/ccc/zzz", reply::ok);
  test_request("/foo/bar/xxx", reply::ok);
  test_request("/foo/xxx", reply::ok);
  test_request("/foo/bar/aaa/zzz", reply::ok);

  test_request("/bar/foo/xxx", reply::not_found, "PUT");

  REQUIRE(calls.size() == 7);

  CHECK(calls[0].first == 3);
  CHECK(calls[0].second == "fooxxx");

  CHECK(calls[1].first == 4);
  CHECK(calls[1].second == "zzz");

  CHECK(calls[2].first == 5);
  CHECK(calls[2].second == "xxx");

  CHECK(calls[3].first == 6);
  CHECK(calls[3].second == "zzz");

  CHECK(calls[4].first == 2);
  CHECK(calls[4].second == "xxx");

  CHECK(calls[5].first == 1);
  CHECK(calls[5].second == "xxx");

  CHECK(calls[6].first == 7);
  CHECK(calls[6].second == "zzz");
}

TEST_CASE("dynamic_content handles request correctly", "[dynamic_content][serve]") // NOLINT
{
  auto handler = get([](const request& req, std::ostream& os) {
    os << "Resource1: " << req.resource("resource1") << "\n";
    os << "Resource2: " << req.resource("resource2") << "\n";
    os << "QueryParam: " << req.query("param") << "\n";
  });

  http_request http_req;
  reply rep;

  SECTION("Handles path and query parameters")
  {
    handler.serve_if_match("/foo/:resource1/:resource2", "/foo/res1/res2?param=value", http_req, rep);

    REQUIRE(rep.status == reply::ok);
    REQUIRE(rep.content == "Resource1: res1\nResource2: res2\nQueryParam: value\n");
    REQUIRE(rep.headers.size() == 2);
    REQUIRE(rep.headers[0].name == "Content-Length");
    REQUIRE(rep.headers[0].value == std::to_string(rep.content.size()));
    REQUIRE(rep.headers[1].name == "Content-Type");
    REQUIRE(rep.headers[1].value == "text/plain");
  }

  SECTION("Handles path without query parameters")
  {
    handler.serve_if_match("/foo/:resource1/:resource2", "/foo/res1/res2", http_req, rep);

    REQUIRE(rep.status == reply::ok);
    REQUIRE(rep.content == "Resource1: res1\nResource2: res2\nQueryParam: \n");
    REQUIRE(rep.headers.size() == 2);
    REQUIRE(rep.headers[0].name == "Content-Length");
    REQUIRE(rep.headers[0].value == std::to_string(rep.content.size()));
    REQUIRE(rep.headers[1].name == "Content-Type");
    REQUIRE(rep.headers[1].value == "text/plain");
  }

  SECTION("Handles query parameters without path")
  {
    handler.serve_if_match("/foo/res", "/foo/res?param=value", http_req, rep);

    REQUIRE(rep.status == reply::ok);
    REQUIRE(rep.content == "Resource1: \nResource2: \nQueryParam: value\n");
    REQUIRE(rep.headers.size() == 2);
    REQUIRE(rep.headers[0].name == "Content-Length");
    REQUIRE(rep.headers[0].value == std::to_string(rep.content.size()));
    REQUIRE(rep.headers[1].name == "Content-Type");
    REQUIRE(rep.headers[1].value == "text/plain");
  }
}