// Copyright (c) 2022 Daniele Pallastrelli
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

#include "logger_collection.hpp"

using namespace f16::http::server;

TEST_CASE("logger_collection supports syslog sink with string options", "[logger_collection][syslog]")
{
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {
            {"type", "syslog"},
            {"ident", "f16-test"},
            {"facility", "user"},
            {"option", "pid"},
            {"enable_formatting", false}
          }
        })}
      }},
      {"error", {
        {"enabled", false}
      }}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("syslog_string_options");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection supports syslog sink with numeric and array options", "[logger_collection][syslog]")
{
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {
            {"type", "syslog"},
            {"ident", "f16-test"},
            {"facility", 8},
            {"options", nlohmann::json::array({"pid", "cons"})},
            {"enable_formatting", false}
          }
        })}
      }},
      {"error", {
        {"enabled", false}
      }}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("syslog_numeric_options");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection skips syslog sink with invalid facility string", "[logger_collection][syslog]")
{
  // Invalid facility: syslog sink should be skipped with warning; logger must still be created.
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {
            {"type", "syslog"},
            {"facility", "invalid_facility"}
          }
        })}
      }},
      {"error", {
        {"enabled", false}
      }}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("syslog_invalid_facility");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection skips syslog sink with invalid option string", "[logger_collection][syslog]")
{
  // Invalid option: syslog sink should be skipped with warning; logger must still be created.
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {
            {"type", "syslog"},
            {"facility", "user"},
            {"option", "invalid_option"}
          }
        })}
      }},
      {"error", {
        {"enabled", false}
      }}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("syslog_invalid_option");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection skips syslog sink with options field not being an array", "[logger_collection][syslog]")
{
  // options must be an array: syslog sink should be skipped with warning; logger must still be created.
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {
            {"type", "syslog"},
            {"facility", "user"},
            {"options", "pid"}    // string instead of array
          }
        })}
      }},
      {"error", {
        {"enabled", false}
      }}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("syslog_invalid_options_type");

  REQUIRE(logger != nullptr);
}

// ---------------------------------------------------------------------------
// stdout sink
// ---------------------------------------------------------------------------

TEST_CASE("logger_collection supports stdout sink with color", "[logger_collection][stdout]")
{
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {{"type", "stdout"}, {"color", true}, {"level", "info"}}
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("stdout_color");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection supports stdout sink without color", "[logger_collection][stdout]")
{
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {{"type", "stdout"}, {"color", false}}
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("stdout_no_color");

  REQUIRE(logger != nullptr);
}

// ---------------------------------------------------------------------------
// stderr sink
// ---------------------------------------------------------------------------

TEST_CASE("logger_collection supports stderr sink with color", "[logger_collection][stderr]")
{
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {{"type", "stderr"}, {"color", true}, {"level", "warn"}}
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("stderr_color");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection supports stderr sink without color", "[logger_collection][stderr]")
{
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {{"type", "stderr"}, {"color", false}}
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("stderr_no_color");

  REQUIRE(logger != nullptr);
}

// ---------------------------------------------------------------------------
// file sink
// ---------------------------------------------------------------------------

TEST_CASE("logger_collection supports file sink with valid path", "[logger_collection][file]")
{
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {
            {"type", "file"},
            {"path", "/tmp/f16_test_access.log"},
            {"max_size", 1048576},
            {"max_files", 3},
            {"level", "info"}
          }
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("file_valid_path");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection skips file sink with empty path", "[logger_collection][file]")
{
  // Empty path: file sink must be skipped; logger must still be created.
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {{"type", "file"}, {"path", ""}}
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("file_empty_path");

  REQUIRE(logger != nullptr);
}

TEST_CASE("logger_collection skips file sink with missing path key", "[logger_collection][file]")
{
  // Missing path key: file sink must be skipped; logger must still be created.
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {{"type", "file"}}    // no "path"
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("file_missing_path");

  REQUIRE(logger != nullptr);
}

// ---------------------------------------------------------------------------
// unknown sink type
// ---------------------------------------------------------------------------

TEST_CASE("logger_collection skips unknown sink type", "[logger_collection]")
{
  // Unknown type must be skipped with warning; logger must still be created.
  const nlohmann::json config = {
    {"logs", {
      {"access", {
        {"enabled", true},
        {"sinks", nlohmann::json::array({
          {{"type", "nonexistent_sink"}}
        })}
      }},
      {"error", {{"enabled", false}}}
    }}
  };

  logger_collection loggers(config);
  const auto logger = loggers.create_logger_from_config("unknown_sink_type");

  REQUIRE(logger != nullptr);
}
