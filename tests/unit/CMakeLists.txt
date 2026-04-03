find_package(Catch2 REQUIRED)

include(CTest)
include(Catch)

add_library(catch_main OBJECT catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)
target_link_libraries(catch_main PRIVATE f16_project_options)

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
add_test(NAME cli.has_help COMMAND f16 --help)

# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
add_test(NAME cli.version_matches COMMAND f16 --version)
set_tests_properties(cli.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")


add_executable(tests tests.cpp)
target_link_libraries(tests 
  PRIVATE
    f16_project_options f16_project_warnings
    catch_main
    f16lib
  )

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries

# if(Catch2_VERSION VERSION_LESS "3.0.0")
#   set(TEST_REPORTER junit)
# else()
#   set(TEST_REPORTER xml)
# endif()
# 
# if (CATCH_VERSION VERSION_GREATER_EQUAL 3)
#     set(TEST_REPORTER "xml")
# else()
#     set(TEST_REPORTER "junit")
# endif()

catch_discover_tests(
  tests
  TEST_PREFIX "unittests."
  # REPORTER ${TEST_REPORTER}
  OUTPUT_DIR .
  OUTPUT_PREFIX "unittests."
  OUTPUT_SUFFIX .xml
)