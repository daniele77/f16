// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "reply.hpp"
#include <string>
#include <unordered_map>

namespace f16::http::server {

namespace status_strings {

static const std::string ok = // NOLINT
  "HTTP/1.0 200 OK\r\n";
static const std::string created = // NOLINT
  "HTTP/1.0 201 Created\r\n";
static const std::string accepted = // NOLINT
  "HTTP/1.0 202 Accepted\r\n";
static const std::string no_content = // NOLINT
  "HTTP/1.0 204 No Content\r\n";
static const std::string multiple_choices = // NOLINT
  "HTTP/1.0 300 Multiple Choices\r\n";
static const std::string moved_permanently = // NOLINT
  "HTTP/1.0 301 Moved Permanently\r\n";
static const std::string moved_temporarily = // NOLINT
  "HTTP/1.0 302 Moved Temporarily\r\n";
static const std::string not_modified = // NOLINT
  "HTTP/1.0 304 Not Modified\r\n";
static const std::string bad_request = // NOLINT
  "HTTP/1.0 400 Bad Request\r\n";
static const std::string unauthorized = // NOLINT
  "HTTP/1.0 401 Unauthorized\r\n";
static const std::string forbidden = // NOLINT
  "HTTP/1.0 403 Forbidden\r\n";
static const std::string not_found = // NOLINT
  "HTTP/1.0 404 Not Found\r\n";
static const std::string internal_server_error = // NOLINT
  "HTTP/1.0 500 Internal Server Error\r\n";
static const std::string not_implemented = // NOLINT
  "HTTP/1.0 501 Not Implemented\r\n";
static const std::string bad_gateway = // NOLINT
  "HTTP/1.0 502 Bad Gateway\r\n";
static const std::string service_unavailable = // NOLINT
  "HTTP/1.0 503 Service Unavailable\r\n";
static const std::string gateway_timeout = // NOLINT
  "HTTP/1.0 504 Gateway Timeout\r\n";
static const std::string http_version_not_supported = // NOLINT
  "HTTP/1.0 505 HTTP Version Not Supported\r\n";

static asio::const_buffer to_buffer(reply::status_type status)
{
  switch (status)
  {
  case reply::ok:
    return asio::buffer(ok);
  case reply::created:
    return asio::buffer(created);
  case reply::accepted:
    return asio::buffer(accepted);
  case reply::no_content:
    return asio::buffer(no_content);
  case reply::multiple_choices:
    return asio::buffer(multiple_choices);
  case reply::moved_permanently:
    return asio::buffer(moved_permanently);
  case reply::moved_temporarily:
    return asio::buffer(moved_temporarily);
  case reply::not_modified:
    return asio::buffer(not_modified);
  case reply::bad_request:
    return asio::buffer(bad_request);
  case reply::unauthorized:
    return asio::buffer(unauthorized);
  case reply::forbidden:
    return asio::buffer(forbidden);
  case reply::not_found:
    return asio::buffer(not_found);
  case reply::internal_server_error:
    return asio::buffer(internal_server_error);
  case reply::not_implemented:
    return asio::buffer(not_implemented);
  case reply::bad_gateway:
    return asio::buffer(bad_gateway);
  case reply::service_unavailable:
    return asio::buffer(service_unavailable);
  case reply::gateway_timeout:
    return asio::buffer(gateway_timeout);
  case reply::http_version_not_supported:
    return asio::buffer(http_version_not_supported);
  default:
    return asio::buffer(internal_server_error);
  }
}

} // namespace status_strings

namespace misc_strings {

static const std::string name_value_separator = ": "; // NOLINT
static const std::string crlf = "\r\n"; // NOLINT

} // namespace misc_strings

std::vector<asio::const_buffer> reply::to_buffers()
{
  std::vector<asio::const_buffer> buffers;
  buffers.push_back(status_strings::to_buffer(status));
  for (const header& h: headers)
  {
    buffers.push_back(asio::buffer(h.name));
    buffers.push_back(asio::buffer(misc_strings::name_value_separator));
    buffers.push_back(asio::buffer(h.value));
    buffers.push_back(asio::buffer(misc_strings::crlf));
  }
  buffers.push_back(asio::buffer(misc_strings::crlf));
  buffers.push_back(asio::buffer(content)); // NOLINT
  return buffers;
}

namespace stock_replies {

static const std::string ok = ""; // NOLINT
static const std::string created = // NOLINT
  "<html>"
  "<head><title>Created</title></head>"
  "<body><h1>201 Created</h1></body>"
  "</html>";
static const std::string accepted = // NOLINT
  "<html>"
  "<head><title>Accepted</title></head>"
  "<body><h1>202 Accepted</h1></body>"
  "</html>";
static const std::string no_content = // NOLINT
  "<html>"
  "<head><title>No Content</title></head>"
  "<body><h1>204 Content</h1></body>"
  "</html>";
static const std::string multiple_choices = // NOLINT
  "<html>"
  "<head><title>Multiple Choices</title></head>"
  "<body><h1>300 Multiple Choices</h1></body>"
  "</html>";
static const std::string moved_permanently = // NOLINT
  "<html>"
  "<head><title>Moved Permanently</title></head>"
  "<body><h1>301 Moved Permanently</h1></body>"
  "</html>";
static const std::string moved_temporarily = // NOLINT
  "<html>"
  "<head><title>Moved Temporarily</title></head>"
  "<body><h1>302 Moved Temporarily</h1></body>"
  "</html>";
static const std::string not_modified = // NOLINT
  "<html>"
  "<head><title>Not Modified</title></head>"
  "<body><h1>304 Not Modified</h1></body>"
  "</html>";
static const std::string bad_request = // NOLINT
  "<html>"
  "<head><title>Bad Request</title></head>"
  "<body><h1>400 Bad Request</h1></body>"
  "</html>";
static const std::string unauthorized = // NOLINT
  "<html>"
  "<head><title>Unauthorized</title></head>"
  "<body><h1>401 Unauthorized</h1></body>"
  "</html>";
static const std::string forbidden = // NOLINT
  "<html>"
  "<head><title>Forbidden</title></head>"
  "<body><h1>403 Forbidden</h1></body>"
  "</html>";
static const std::string not_found = // NOLINT
  "<html>"
  "<head><title>Not Found</title></head>"
  "<body><h1>404 Not Found</h1></body>"
  "</html>";
static const std::string internal_server_error = // NOLINT
  "<html>"
  "<head><title>Internal Server Error</title></head>"
  "<body><h1>500 Internal Server Error</h1></body>"
  "</html>";
static const std::string not_implemented = // NOLINT
  "<html>"
  "<head><title>Not Implemented</title></head>"
  "<body><h1>501 Not Implemented</h1></body>"
  "</html>";
static const std::string bad_gateway = // NOLINT
  "<html>"
  "<head><title>Bad Gateway</title></head>"
  "<body><h1>502 Bad Gateway</h1></body>"
  "</html>";
static const std::string service_unavailable = // NOLINT
  "<html>"
  "<head><title>Service Unavailable</title></head>"
  "<body><h1>503 Service Unavailable</h1></body>"
  "</html>";
static const std::string gateway_timeout = // NOLINT
  "<html>"
  "<head><title>Gateway Timeout</title></head>"
  "<body><h1>504 Gateway Timeout</h1></body>"
  "</html>";
static const std::string http_version_not_supported = // NOLINT
  "<html>"
  "<head><title>HTTP Version Not Supported</title></head>"
  "<body><h1>505 HTTP Version Not Supported</h1></body>"
  "</html>";

static std::string to_string(reply::status_type status)
{
  switch (status)
  {
  case reply::ok:
    return ok;
  case reply::created:
    return created;
  case reply::accepted:
    return accepted;
  case reply::no_content:
    return no_content;
  case reply::multiple_choices:
    return multiple_choices;
  case reply::moved_permanently:
    return moved_permanently;
  case reply::moved_temporarily:
    return moved_temporarily;
  case reply::not_modified:
    return not_modified;
  case reply::bad_request:
    return bad_request;
  case reply::unauthorized:
    return unauthorized;
  case reply::forbidden:
    return forbidden;
  case reply::not_found:
    return not_found;
  case reply::internal_server_error:
    return internal_server_error;
  case reply::not_implemented:
    return not_implemented;
  case reply::bad_gateway:
    return bad_gateway;
  case reply::service_unavailable:
    return service_unavailable;
  default:
    return internal_server_error;
  }
}

} // namespace stock_replies

reply reply::stock_reply(reply::status_type status)
{
  reply rep;
  rep.status = status;
  rep.content = stock_replies::to_string(status);
  rep.headers = {
    {"Content-Length", std::to_string(rep.content.size())},
    {"Content-Type", "text/html"}
  };
  return rep;
}

reply::status_type reply::status_from_string(const std::string& s)
{
  static const std::unordered_map<std::string, reply::status_type> status_map = {
    {"ok", reply::ok},
    {"created", reply::created},
    {"accepted", reply::accepted},
    {"no_content", reply::no_content},
    {"multiple_choices", reply::multiple_choices},
    {"moved_permanently", reply::moved_permanently},
    {"moved_temporarily", reply::moved_temporarily},
    {"not_modified", reply::not_modified},
    {"bad_request", reply::bad_request},
    {"unauthorized", reply::unauthorized},
    {"forbidden", reply::forbidden},
    {"not_found", reply::not_found},
    {"internal_server_error", reply::internal_server_error},
    {"not_implemented", reply::not_implemented},
    {"bad_gateway", reply::bad_gateway},
    {"service_unavailable", reply::service_unavailable},
    {"gateway_timeout", reply::gateway_timeout},
    {"http_version_not_supported", reply::http_version_not_supported}
  };
  auto it = status_map.find(s);
  if (it != status_map.end())
    return it->second;
  throw std::invalid_argument("Unknown status: " + s);
}

} // namespace f16::http::server
