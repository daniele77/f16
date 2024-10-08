// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_CONNECTION_MANAGER_HPP
#define F16_HTTP_CONNECTION_MANAGER_HPP

#include <unordered_set>
#include "connection.hpp"

namespace f16::http::server {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager
{
public:
  connection_manager(const connection_manager&) = delete;
  connection_manager& operator=(const connection_manager&) = delete;

  /// Construct a connection manager.
  connection_manager();
  
  ~connection_manager();

  /// Add the specified connection to the manager and start it.
  void start(const connection_ptr& c);

  /// Stop the specified connection.
  void stop(const connection_ptr& c);

private:
  /// Stop all connections.
  void stop_all();

  /// The managed connections.
  std::unordered_set<connection_ptr> connections_;
};

} // namespace f16::http::server

#endif // F16_HTTP_CONNECTION_MANAGER_HPP
