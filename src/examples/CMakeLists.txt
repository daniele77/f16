set(EXAMPLES
  static
  rest
  complete
)

find_package(asio CONFIG)

foreach(example ${EXAMPLES})
  add_executable(example-${example} ${example}.cpp)
  target_link_libraries(
    example-${example}
    PRIVATE project_options
            project_warnings
            f16lib
  )
endforeach()
