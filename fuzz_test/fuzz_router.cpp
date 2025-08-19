// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <cstdint>
#include "path_router.hpp"
#include "http_request.hpp"
#include "reply.hpp"

using namespace f16::http::server;

static path_router& GetRouter()
{
  static path_router router;
  router.add("/foo", get([](const request&, std::ostream&){}));
  router.add("/foo/bar", get([](const request&, std::ostream&){}));
  router.add("/bar", get([](const request&, std::ostream&){}));
  router.add("/bar/foo/aaa", get([](const request&, std::ostream&){}));
  router.add("/bar/foo/bbb", get([](const request&, std::ostream&){}));
  router.add("/bar/foo/bbb/ccc", get([](const request&, std::ostream&){}));
  router.add("/foo/bar/aaa", get([](const request&, std::ostream&){}));
  return router;
}

[[nodiscard]] auto GetPath(const uint8_t* data, size_t size)
{
  const std::string path(reinterpret_cast<const char *>(data), size); // NOLINT
  return path;
}

// cppcheck-suppress unusedFunction symbolName=LLVMFuzzerTestOneInput
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
  static path_router& router = GetRouter();
  static http_request req;
  static reply rep;

  req.method = "GET"; // Default method
  req.http_version_major = 1;
  req.http_version_minor = 1;
  req.headers.clear();

  const auto path = GetPath(Data, Size);
  req.uri = path;
  router(req, rep);

  return 0;
}
