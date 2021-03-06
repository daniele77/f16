// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_MIME_TYPES_HPP
#define F16_HTTP_MIME_TYPES_HPP

#include <string>

namespace f16::http::server::mime_types {

/// Convert a file extension into a MIME type.
std::string extension_to_type(const std::string& extension);

} // namespace f16::http::server::mime_types

#endif // F16_HTTP_MIME_TYPES_HPP
