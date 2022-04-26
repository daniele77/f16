add_executable(f16 main.cpp)
target_link_libraries(
  f16
  PRIVATE project_options
          project_warnings
          # docopt::docopt
          docopt_s
          # fmt::fmt  already in spdlog
          spdlog::spdlog)

target_include_directories(f16 PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

