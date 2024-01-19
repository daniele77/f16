set(EXAMPLES
  static
  rest
  complete
)

foreach(example ${EXAMPLES})
  add_executable(example-${example} ${example}.cpp)
  target_link_libraries(
    example-${example}
    PRIVATE
      f16lib
  )
endforeach()
