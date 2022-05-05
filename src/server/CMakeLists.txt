find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)
find_package(asio CONFIG)

add_executable(f16
  main.cpp)
target_link_libraries(
  f16
  PRIVATE project_options
          project_warnings
          f16lib
          docopt::docopt
          fmt::fmt
          spdlog::spdlog)
# target_link_system_libraries(
#   f16
#   PRIVATE asio::asio)

target_include_directories(f16 PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

