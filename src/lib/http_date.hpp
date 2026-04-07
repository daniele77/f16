// Copyright (c) 2026 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef F16_HTTP_DATE_HPP
#define F16_HTTP_DATE_HPP

#include <chrono>
#include <ctime>
#include <array>
#include <string>
#include <cstdio>

inline std::tm gmtime_utc(std::time_t t) noexcept
{
  std::tm tm{};
#if defined(_WIN32)
  gmtime_s(&tm, &t);
#else
  gmtime_r(&t, &tm);
#endif
  return tm;
}
/**
 * @brief Returns the current date and time formatted according to RFC 7231.
 */
inline std::string http_date()
{
  using namespace std::chrono;

  // Thread-local cache (lock-free by design)
  thread_local std::time_t cached_sec = 0;
  thread_local std::string cached_str;

  const auto now  = system_clock::now();
  const auto secs = time_point_cast<seconds>(now);
  const std::time_t tt = system_clock::to_time_t(secs);

  // Fast path: same second → return cached string
  if (tt == cached_sec) {
      return cached_str;
  }

  // Slow path: recompute
  cached_sec = tt;

  const std::tm tm = gmtime_utc(tt);

  // RFC 7231 fixed English abbreviations (locale-independent)
  static constexpr std::array<const char*, 7> wday = {
      "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
  };

  static constexpr std::array<const char*, 12> mon = {
      "Jan","Feb","Mar","Apr","May","Jun",
      "Jul","Aug","Sep","Oct","Nov","Dec"
  };

  std::array<char, 30> buf;

  const int len = std::snprintf(buf.data(), buf.size(),
      "%s, %02d %s %04d %02d:%02d:%02d GMT",
      wday[static_cast<std::size_t>(tm.tm_wday)],
      tm.tm_mday,
      mon[static_cast<std::size_t>(tm.tm_mon)],
      tm.tm_year + 1900,
      tm.tm_hour,
      tm.tm_min,
      tm.tm_sec
  );

  // Update cached string (at most once per second per thread)
  cached_str.assign(buf.data(), static_cast<std::size_t>(len));

  return cached_str;
}

#endif // F16_HTTP_DATE_HPP