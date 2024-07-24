// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_CONNECTION_HPP
#define F16_HTTP_CONNECTION_HPP

#include <memory>

namespace f16::http::server {

class connection_manager;

/// Represents a single connection from a client.
class connection
{
public:

  virtual ~connection() noexcept = default;

  /// Start the first asynchronous operation for the connection.
  virtual void start() = 0;

  /// Stop all asynchronous operations associated with the connection.
  virtual void stop() = 0;
};

using connection_ptr = std::shared_ptr<connection>;

/*
template <typename Connection>
class connection_wrapper : public connection
{
    template <typename... Args>
    connection_wrapper(Args&&... args) : c{std::forward<Args>(args)...} {}
    void start() override { c.start(); }
    void stop() override { c.stop(); }
    Connection c;
};
*/

} // namespace f16::http::server

#endif // F16_HTTP_CONNECTION_HPP
