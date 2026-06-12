set(FS_LINK_FLAG "")
if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0) OR
   ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0))
    set(FS_LINK_FLAG "-lstdc++fs")
endif()

add_library(f16lib
  url.hpp url.cpp
  connection.hpp
  base_connection.hpp
  plain_connection.hpp plain_connection.cpp
  ssl_connection.hpp ssl_connection.cpp
  connection_manager.hpp connection_manager.cpp
  mime_types.cpp
  reply.cpp
  request_handler.hpp request_handler.cpp
  request_parser.hpp request_parser.cpp
  http_server.cpp
  https_server.cpp
  path_router.cpp
  static_content.cpp
  dynamic_content.cpp
)

# link project_options/warnings
target_link_libraries(f16lib
  PRIVATE f16_project_options f16_project_warnings
)

target_include_directories(f16lib
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Threads REQUIRED)

target_link_libraries(
  f16lib
  PRIVATE
    ${FS_LINK_FLAG}
    Threads::Threads
  PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    asio::asio
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/f16Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/f16Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/f16
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/f16ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(TARGETS f16lib
  EXPORT f16libTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/f16
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT f16libTargets
  FILE f16libTargets.cmake
  NAMESPACE f16::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/f16
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/f16Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/f16ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/f16
)
