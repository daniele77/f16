find_package(asio CONFIG)

add_library(f16lib
  connection.cpp
  connection_manager.cpp
  mime_types.cpp
  reply.cpp
  request_handler.cpp
  request_parser.cpp
  server.cpp)
target_include_directories(f16lib INTERFACE .)
target_link_libraries(
  f16lib
  PRIVATE project_options
          project_warnings
          -lstdc++fs)
target_link_system_libraries(
  f16lib
  PRIVATE asio::asio)

