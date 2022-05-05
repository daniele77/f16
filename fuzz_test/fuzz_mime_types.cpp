// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include "mime_types.hpp"

[[nodiscard]] auto GetExtension(const uint8_t* data, size_t size)
{
  const std::string ext(reinterpret_cast<const char *>(data), size); // NOLINT
  return f16::http::server::mime_types::extension_to_type(ext);
}

// cppcheck-suppress unusedFunction symbolName=LLVMFuzzerTestOneInput
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
  const auto ext = GetExtension(Data, Size);
  if (ext.empty())
    __builtin_trap();

  return 0;
}
