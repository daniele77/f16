# A fuzz test runs until it finds an error. This particular one is going to rely on libFuzzer.
#

set(FUZZ_TESTS
  fuzz_mime_types
  fuzz_request_parser
  fuzz_router
)

# Allow short runs during automated testing to see if something new breaks
set(FUZZ_RUNTIME
    10
    CACHE STRING "Number of seconds to run fuzz tests during ctest run") # Default of 10 seconds

foreach(test ${FUZZ_TESTS})
  add_executable(${test} ${test}.cpp)
  target_link_libraries(
    ${test}
    PRIVATE
      f16_project_options f16_project_warnings
      f16lib
      -coverage
      -fsanitize=fuzzer,undefined,address)
  target_compile_options(${test} PRIVATE -fsanitize=fuzzer,undefined,address)
  add_test(NAME ${test}_run COMMAND ${test} -max_total_time=${FUZZ_RUNTIME})
endforeach()
