find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(nlohmann_json CONFIG)
find_package(CLI11 REQUIRED)

add_executable(f16
main.cpp)

# link project_options/warnings
target_link_libraries(f16
PRIVATE f16_project_options f16_project_warnings
)

target_link_libraries(
  f16
  PRIVATE
  f16lib
  )
  
target_link_system_libraries(
  f16
  PRIVATE
    CLI11::CLI11
    fmt::fmt
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

target_include_directories(f16 PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

