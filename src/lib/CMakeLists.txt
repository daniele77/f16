set(FS_LINK_FLAG "")
if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0) OR
   ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0))
    set(FS_LINK_FLAG "-lstdc++fs")
endif()

add_library(f16lib
  connection.hpp connection.cpp
  connection_manager.hpp connection_manager.cpp
  mime_types.hpp mime_types.cpp
  reply.hpp reply.cpp
  request_handler.hpp request_handler.cpp
  request_parser.hpp request_parser.cpp
  server.hpp server.cpp
  path_router.hpp path_router.cpp
  static_content.hpp static_content.cpp
  dynamic_content_get.hpp dynamic_content_get.cpp
  http_handler.hpp
)

# link project_options/warnings
target_link_libraries(f16lib
  PRIVATE f16_project_options f16_project_warnings
)

target_include_directories(f16lib SYSTEM INTERFACE . ${STANDALONE_ASIO_INCLUDE_PATH})

target_link_libraries(
  f16lib
  PRIVATE
    ${FS_LINK_FLAG}
  PUBLIC
    asio::asio
)
