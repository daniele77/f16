find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(
  NAME blackbox.http_server
  COMMAND
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_blackbox.py
    --app
    $<TARGET_FILE:example-blackbox_app>
    --host
    127.0.0.1
    --port
    0
    --doc-root
    ${CMAKE_CURRENT_SOURCE_DIR}/www
)

set_tests_properties(blackbox.http_server PROPERTIES TIMEOUT 30)