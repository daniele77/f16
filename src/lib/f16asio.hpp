// Copyright (c) 2024 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_F16ASIO_HPP
#define F16_F16ASIO_HPP

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#include <asio.hpp>
#pragma GCC diagnostic pop
#else
#include <asio.hpp>
#endif

#endif // F16_F16ASIO_HPP
