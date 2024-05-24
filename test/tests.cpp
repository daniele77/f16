// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request_parser.hpp"
#include "http_request.hpp"
#include "path_router.hpp"
#include "dynamic_content.hpp"

using namespace f16::http::server;

TEST_CASE("mime types are deduced by extension", "[mime_types]")
{
  using namespace f16::http::server::mime_types;
  REQUIRE(extension_to_type(".gif") == "image/gif");
  REQUIRE(extension_to_type(".htm") == "text/html" );
  REQUIRE(extension_to_type(".html") == "text/html" );
  REQUIRE(extension_to_type(".jpg") == "image/jpeg" );
  REQUIRE(extension_to_type(".png") == "image/png");
  REQUIRE(extension_to_type(".unk") == "text/plain");
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


class dummy_handler : public http_handler
{
public:
  explicit dummy_handler(int _id) : id(_id) {} 
  void serve(const std::string& resource, const http_request& /* req */, reply& /* rep */) override
  { 
	  calls.emplace_back(id, resource);
  }
  [[nodiscard]] std::string method() const override { return "GET"; }
  static std::vector<std::pair<int, std::string>> calls;
private:  
  const int id;
};

std::vector<std::pair<int, std::string>> dummy_handler::calls;


TEST_CASE("path_router routes simple requests", "[path_router]") // NOLINT
{ 
  path_router router; 
  router.add("/foo", std::make_shared<dummy_handler>(1));
  router.add("/foo/bar", std::make_shared<dummy_handler>(2));
  router.add("/bar", std::make_shared<dummy_handler>(3));
  router.add("/bar/foo/aaa", std::make_shared<dummy_handler>(4));
  router.add("/bar/foo/bbb", std::make_shared<dummy_handler>(5));
  router.add("/bar/foo/bbb/ccc", std::make_shared<dummy_handler>(6));
  router.add("/foo/bar/aaa", std::make_shared<dummy_handler>(7));

  http_request req;
  req.method = "GET";
  reply rep;

  CHECK_FALSE( router.serve("/ddd", req, rep) );
  CHECK( router.serve("/bar/foo/xxx", req, rep) );
  CHECK( router.serve("/bar/foo/aaa/zzz", req, rep) );
  CHECK( router.serve("/bar/foo/bbb/xxx", req, rep) );
  CHECK( router.serve("/bar/foo/bbb/ccc/zzz", req, rep) );
  CHECK( router.serve("/foo/bar/xxx", req, rep) );
  CHECK( router.serve("/foo/xxx", req, rep) );
  CHECK( router.serve("/foo/bar/aaa/zzz", req, rep) );

  req.method = "PUT";
  CHECK_FALSE( router.serve("/bar/foo/xxx", req, rep) );

  REQUIRE(dummy_handler::calls.size() == 7);
  
  CHECK(dummy_handler::calls[0].first == 3);
  CHECK(dummy_handler::calls[0].second == "/foo/xxx");

  CHECK(dummy_handler::calls[1].first == 4);
  CHECK(dummy_handler::calls[1].second == "/zzz");

  CHECK(dummy_handler::calls[2].first == 5);
  CHECK(dummy_handler::calls[2].second == "/xxx");

  CHECK(dummy_handler::calls[3].first == 6);
  CHECK(dummy_handler::calls[3].second == "/zzz");

  CHECK(dummy_handler::calls[4].first == 2);
  CHECK(dummy_handler::calls[4].second == "/xxx");

  CHECK(dummy_handler::calls[5].first == 1);
  CHECK(dummy_handler::calls[5].second == "/xxx");

  CHECK(dummy_handler::calls[6].first == 7);
  CHECK(dummy_handler::calls[6].second == "/zzz");
}

TEST_CASE("dynamic_content calls handler", "[dynamic_content]") // NOLINT
{
  static const std::string output{"content"};
  auto g = get([](const request&, std::ostream& o){
    o << output;
  });
  CHECK(g->method() == "GET");
  http_request req;
  req.method = "GET";
  reply rep;
  g->serve("/param1/param2", req, rep);
  CHECK(rep.content == output);
  REQUIRE(rep.headers.size() == 2);
  CHECK(rep.headers[0].name == "Content-Length");
  CHECK(rep.headers[0].value == std::to_string(output.size()));
  CHECK(rep.headers[1].name == "Content-Type");
  CHECK(rep.headers[1].value == mime_types::extension_to_type("txt"));

  g = get({"key1", "key2"}, [](const request& r, std::ostream& o){
    o << output;
    CHECK(r.param("key1") == "param1");
    CHECK(r.param("key2") == "param2");
  });
  CHECK(g->method() == "GET");
  req.method = "GET";
  g->serve("/param1/param2", req, rep);
  CHECK(rep.content == output);
  REQUIRE(rep.headers.size() == 2);
  CHECK(rep.headers[0].name == "Content-Length");
  CHECK(rep.headers[0].value == std::to_string(output.size()));
  CHECK(rep.headers[1].name == "Content-Type");
  CHECK(rep.headers[1].value == mime_types::extension_to_type("txt"));
}