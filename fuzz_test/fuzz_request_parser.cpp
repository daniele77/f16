// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include "request_parser.hpp"
#include "request.hpp"

// cppcheck-suppress unusedFunction symbolName=LLVMFuzzerTestOneInput
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  using namespace f16::http::server;

  request_parser grammar;
  request req;
  const std::string input(reinterpret_cast<const char *>(data), size); // NOLINT
  grammar.parse(req, input.begin(), input.end());

  return 0;
}
