set(EXAMPLES
  static
  rest
)

find_package(asio CONFIG)

foreach(example ${EXAMPLES})
  add_executable(example-${example} ${example}.cpp)
  list(APPEND EXAMPLE_TARGETS example-${example})
  target_link_libraries(
    example-${example}
    PRIVATE project_options
            project_warnings
            f16lib
            asio::asio)
endforeach()
