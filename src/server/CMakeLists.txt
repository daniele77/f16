find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(nlohmann_json CONFIG)

# load DOCOPT
find_package(docopt CONFIG)
include_directories(${DOCOPT_INCLUDE_DIRS})

add_executable(f16
  main.cpp)
target_link_libraries(
  f16
  PRIVATE
    f16lib
    docopt_s
    fmt::fmt
    spdlog::spdlog
    nlohmann_json::nlohmann_json)

target_include_directories(f16 PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

