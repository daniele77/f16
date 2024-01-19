find_package(asio 1.24.0 QUIET)
if(NOT asio_FOUND)
  find_path(STANDALONE_ASIO_INCLUDE_PATH NAMES "asio.hpp" HINTS ${ASIO_INCLUDEDIR})
  if(STANDALONE_ASIO_INCLUDE_PATH)
    message(STATUS "ASIO library found in ${STANDALONE_ASIO_INCLUDE_PATH}")
    mark_as_advanced(STANDALONE_ASIO_INCLUDE_PATH)
    add_library(asio::asio INTERFACE IMPORTED)
  else()
    message(FATAL_ERROR "ASIO library not found")
  endif()
endif()

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

target_include_directories(f16lib INTERFACE . ${STANDALONE_ASIO_INCLUDE_PATH})
target_link_libraries(
  f16lib
  PRIVATE
    ${FS_LINK_FLAG}
  PUBLIC
    asio::asio
)
