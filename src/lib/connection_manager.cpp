// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "connection_manager.hpp"

namespace f16 {
namespace http {
namespace server {

connection_manager::connection_manager() = default;

void connection_manager::start(const connection_ptr& c)
{
  connections_.insert(c);
  c->start();
}

void connection_manager::stop(const connection_ptr& c)
{
  connections_.erase(c);
  c->stop();
}

void connection_manager::stop_all()
{
  for (const auto& c: connections_)
    c->stop();
  connections_.clear();
}

} // namespace server
} // namespace http
} // namespace f16
