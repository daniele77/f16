// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "path_router.hpp"
#include "http_handler.hpp"
#include "request.hpp"
#include "reply.hpp"

using namespace f16::http::server;

class dummy_handler : public http_handler
{
public:
  void serve(const std::string& /*request_path*/, const request& /*req*/, reply& /*rep*/) override {}
  std::string method() const override { return "GET"; }
};

static path_router* GetRouter()
{
  static path_router router;
  router.add("/foo", std::make_shared<dummy_handler>());
  router.add("/foo/bar", std::make_shared<dummy_handler>());
  router.add("/bar", std::make_shared<dummy_handler>());
  router.add("/bar/foo/aaa", std::make_shared<dummy_handler>());
  router.add("/bar/foo/bbb", std::make_shared<dummy_handler>());
  router.add("/bar/foo/bbb/ccc", std::make_shared<dummy_handler>());
  router.add("/foo/bar/aaa", std::make_shared<dummy_handler>());
  return &router;
}

[[nodiscard]] auto GetPath(const uint8_t* data, size_t size)
{
  const std::string path(reinterpret_cast<const char *>(data), size); // NOLINT
  return path;
}

// cppcheck-suppress unusedFunction symbolName=LLVMFuzzerTestOneInput
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
  static path_router* router = GetRouter();
  static request req;
  static reply rep;

  const auto path = GetPath(Data, Size);
  router->serve(path, req, rep);

  return 0;
}
