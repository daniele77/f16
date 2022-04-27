#include <functional>
#include <iostream>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

// This file will be generated automatically when you run the CMake configuration step.
// It creates a namespace called `f16`.
// You can modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

static constexpr auto USAGE =
  R"(F16.

    Usage:
          f16 ship new <name>...
          f16 ship <name> move <x> <y> [--speed=<kn>]
          f16 ship shoot <x> <y>
          f16 mine (set|remove) <x> <y> [--moored | --drifting]
          f16 (-h | --help)
          f16 --version
 Options:
          -h --help     Show this screen.
          --version     Show version.
          --speed=<kn>  Speed in knots [default: 10].
          --moored      Moored (anchored) mine.
          --drifting    Drifting mine.
)";

int main(int argc, const char **argv)
{
  try {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
      { std::next(argv), std::next(argv, argc) },
      true,// show help if requested
      fmt::format("{} {}",
        f16::cmake::project_name,
        f16::cmake::project_version));// version string, acquired from config.hpp via CMake

    for (auto const &arg : args) { std::cout << arg.first << "=" << arg.second << '\n'; }


    // Use the default logger (stdout, multi-threaded, colored)
    spdlog::info("Hello, {}!", "World");

    fmt::print("Hello, from {}\n", "{fmt}");
  } catch (const std::exception &e) {
    fmt::print("Unhandled exception in main: {}", e.what());
  }
}
