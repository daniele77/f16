find_package(asio CONFIG)

set(FS_LINK_FLAG "")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    set(FS_LINK_FLAG "-lstdc++fs")
  endif()
endif()

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
          ${FS_LINK_FLAG}
  PUBLIC asio::asio
)

# target_link_system_libraries(
#   f16lib
#   PUBLIC asio::asio)

