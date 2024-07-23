set(EXAMPLES
  static
  https_static
  rest
  complete
)

foreach(example ${EXAMPLES})
  add_executable(example-${example} ${example}.cpp)
  target_link_libraries(
    example-${example}
    PRIVATE
      f16_project_options f16_project_warnings
      f16lib
  )
endforeach()
