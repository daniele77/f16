cmake_minimum_required(VERSION 3.8...3.28)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# Include the module to configure the C++ standard
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(SetCppStandard)

include(FetchContent)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

# Add project_options from https://github.com/aminya/project_options
# Change the version in the following URL to update the package (watch the releases of the repository for future updates)
set(PROJECT_OPTIONS_VERSION "v0.35.1")
FetchContent_Declare(_project_options
                     URL https://github.com/aminya/project_options/archive/refs/tags/${PROJECT_OPTIONS_VERSION}.zip)
FetchContent_MakeAvailable(_project_options)
include(${_project_options_SOURCE_DIR}/Index.cmake)

# Set the project name and language
project(
  f16
  VERSION 0.0.1
  DESCRIPTION "f16 C++ web server"
  HOMEPAGE_URL "github.com/daniele77/f16"
  LANGUAGES CXX)

# TODO!!!!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(asio 1.24.0 QUIET)
if(NOT asio_FOUND)
  find_path(
    STANDALONE_ASIO_INCLUDE_PATH
    NAMES "asio.hpp"
    HINTS ${ASIO_INCLUDEDIR})
  if(STANDALONE_ASIO_INCLUDE_PATH)
    message(STATUS "ASIO library found in ${STANDALONE_ASIO_INCLUDE_PATH}")
    mark_as_advanced(STANDALONE_ASIO_INCLUDE_PATH)
    add_library(asio::asio INTERFACE IMPORTED)
    set_property(TARGET asio::asio PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${STANDALONE_ASIO_INCLUDE_PATH}")
  else()
    message(FATAL_ERROR "ASIO library not found")
  endif()
endif()

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# configure files based on CMake configuration options
add_subdirectory(configured_files)

# Adding the src:
add_subdirectory(src)

# Build Features
option(ENABLE_DOCS "Enable the docs" OFF)

# Enable doxgen for the docs
if(ENABLE_DOCS)
  set(ENABLE_DOXYGEN "ENABLE_DOXYGEN")
endif()

option(DISABLE_STACK_PROTECTION "Disable stack protection" OFF)

if(NOT DISABLE_STACK_PROTECTION)
  set(ENABLE_STACK_PROTECTION "ENABLE_STACK_PROTECTION")
endif()

option(DISABLE_ELF_PROTECTION "Disable ELF protection" OFF)

if(NOT DISABLE_ELF_PROTECTION)
  set(ENABLE_ELF_PROTECTION "ENABLE_ELF_PROTECTION")
endif()

# Coverage: default OFF
option(ENABLE_COVERAGE "Enable code coverage (requires -O0, only works on supported platforms)" OFF)

if(ENABLE_COVERAGE)
  set(PROJECT_COVERAGE "ENABLE_COVERAGE")
else()
  set(PROJECT_COVERAGE "")
endif()

# Adding the tests:
option(ENABLE_TESTING "Enable the tests" ON)

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests. testing")
  add_subdirectory(test)
endif()

option(ENABLE_FUZZING "Enable the fuzz tests" OFF)

if(ENABLE_FUZZING)
  message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
  add_subdirectory(fuzz_test)
endif()

option(ENABLE_DEVELOPER_MODE "Enable sanitizers, static analyzers and warning as errors" OFF)
option(ENABLE_THREAD_SANITIZER "Enable thread sanitizer" OFF)

# Enable sanitizers and static analyzers in developer mode
if(ENABLE_DEVELOPER_MODE)

  set(ENABLE_CLANG_TIDY "ENABLE_CLANG_TIDY")
  set(ENABLE_CPPCHECK "ENABLE_CPPCHECK")
  set(WARNINGS_AS_ERRORS "WARNINGS_AS_ERRORS")

  # Controlla quali sanitizzatori sono supportati dal compilatore
  check_sanitizers_support(
    SUPPORTED_ADDRESS
    SUPPORTED_UNDEFINED_BEHAVIOR
    SUPPORTED_LEAK
    SUPPORTED_THREAD
    SUPPORTED_MEMORY
  )

  # Abilita solo quelli supportati
  set(ENABLE_SANITIZER_ADDRESS ${SUPPORTED_ADDRESS})
  set(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR ${SUPPORTED_UNDEFINED_BEHAVIOR})
  set(ENABLE_SANITIZER_LEAK ${SUPPORTED_LEAK})
  set(ENABLE_SANITIZER_THREAD ${SUPPORTED_THREAD})
  set(ENABLE_SANITIZER_MEMORY ${SUPPORTED_MEMORY})

  # Logica ThreadSanitizer: se attivo, disabilita solo quelli che confliggono
  if(ENABLE_THREAD_SANITIZER)
    if(ENABLE_SANITIZER_ADDRESS)
      unset(ENABLE_SANITIZER_ADDRESS)
    endif()
    if(ENABLE_SANITIZER_LEAK)
      unset(ENABLE_SANITIZER_LEAK)
    endif()
  else()
    unset(ENABLE_SANITIZER_THREAD)
  endif()

  message("Developer mode enabled: CLANG_TIDY=${ENABLE_CLANG_TIDY} CPPCHECK=${ENABLE_CPPCHECK} WARNINGS_AS_ERRORS=${WARNINGS_AS_ERRORS} ASAN=${ENABLE_SANITIZER_ADDRESS} UBSAN=${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR} LSan=${ENABLE_SANITIZER_LEAK} TSan=${ENABLE_SANITIZER_THREAD} MSan=${ENABLE_SANITIZER_MEMORY}")

endif()

# Initialize project_options variable related to this project
# This overwrites `project_options` and sets `project_warnings`
# uncomment to enable the options. Some of them accept one or more inputs:
project_options(
  PREFIX "f16"
  ENABLE_CACHE
  ${ENABLE_CPPCHECK}
  ${ENABLE_CLANG_TIDY}
  ENABLE_VS_ANALYSIS
  # ENABLE_CONAN
  # ENABLE_INTERPROCEDURAL_OPTIMIZATION
  # ENABLE_NATIVE_OPTIMIZATION
  ${ENABLE_DOXYGEN}
  ${PROJECT_COVERAGE}
  ${ENABLE_SANITIZER_ADDRESS}
  ${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}
  ${ENABLE_SANITIZER_LEAK}
  ${ENABLE_SANITIZER_THREAD}
  ${ENABLE_SANITIZER_MEMORY}
  ENABLE_CONTROL_FLOW_PROTECTION
  ${ENABLE_STACK_PROTECTION}
  # ENABLE_OVERFLOW_PROTECTION
  ${ENABLE_ELF_PROTECTION}
  # ENABLE_RUNTIME_SYMBOLS_RESOLUTION
  ENABLE_COMPILE_COMMANDS_SYMLINK
  # ENABLE_PCH
  # PCH_HEADERS
  ${WARNINGS_AS_ERRORS}
  # ENABLE_INCLUDE_WHAT_YOU_USE
  # ENABLE_GCC_ANALYZER
  # ENABLE_BUILD_WITH_TIME_TRACE
  # ENABLE_UNITY
  # LINKER "lld"
  # CONAN_PROFILE ${profile_path}
)

if(MSVC)
  # If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
  # so that it behaves well with MSVC's debugger, and we can run the target from visual studio
  get_all_targets(all_targets)
  set_target_properties(${all_targets} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
  # Set the minimum required Windows version to Windows 7
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# set the startup project for the "play" button in MSVC
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT f16)


# Installer and packaging:

install(TARGETS f16 DESTINATION bin)

# Add other files, for example configuration, libraries or documentation
# install(FILES path/to/your/file DESTINATION etc/tuo_progetto)

# CPack configuration
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
# if(WIN32)
#     set(CPACK_GENERATOR "NSIS")
# elseif(UNIX)
#     set(CPACK_GENERATOR "DEB")
#     set(CPACK_PACKAGE_CONTACT "wallaroolib@gmail.com")
#     set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Daniele Pallastrelli")
# endif()
include(CPack)
