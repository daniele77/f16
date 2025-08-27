// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_REQUEST_HPP
#define F16_HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "http_request.hpp"
// #include <iostream> // TODO remove

namespace f16::http::server {

/**
 * @brief A request passed to the handler.
 * 
 * This structure represents a request that is passed to the handler.
 * It contains the original HTTP request and maps for storing path info and query string parameters.
 */
struct request
{
  /**
   * @brief The original HTTP request.
   * 
   * This is the original HTTP request that was received.
   */
  const http_request orig_request;

  /**
   * @brief A map of path info: key -> value.
   * 
   * This map stores information about the path, where each key corresponds to a part of the path.
   */
  std::unordered_map<std::string, std::string> resources;

  /**
   * @brief A map of query string parameters: key -> value.
   * 
   * This map stores the query string parameters of the request.
   */
  std::unordered_map<std::string, std::string> querystring;

  /**
   * @brief Constructs a request with the original HTTP request.
   * 
   * @param r The original HTTP request.
   */
  explicit request(http_request r) : orig_request{std::move(r)} {}

  
  /**
   * @brief Adds a query value to the querystring map.
   * 
   * This function adds a key-value pair to the querystring map.
   * 
   * @param key The key for the query parameter.
   * @param value The value for the query parameter.
   */
  void add_queryvalue(const std::string& key, const std::string& value) { querystring[key] = value; }

  /**
   * @brief Retrieves a resource by key.
   * 
   * This function retrieves the value associated with the specified key in the resources map.
   * 
   * @param key The key for the resource.
   * @return The value associated with the key, empty string if the key was not found
   */
  std::string resource(const std::string& key) const
  {
    auto it = resources.find(key);
    if (it != resources.end()) {
        return it->second;
    }
    return {};
  }

  /**
   * @brief Retrieves a query value by key.
   * 
   * This function retrieves the value associated with the specified key in the querystring map.
   * 
   * @param key The key for the query parameter.
   * @return The value associated with the key, empty string if the key was not found
   */
  std::string query(const std::string& key) const
  {
    auto it = querystring.find(key);
    if (it != querystring.end()) {
        return it->second;
    }
    return {};
  }
};

} // namespace f16::http::server

#endif // F16_HTTP_REQUEST_HPP
