include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
  simple_tracepoint_test

  simple_tracepoint_test.cpp
  test_tp.cpp
)

add_library(
  testtracer

  test_tp.c
)

target_link_libraries(
  simple_tracepoint_test

  ${TESTLIBS}
  testtracer
)

target_link_libraries(
  testtracer

  lttng-ust
  urcu-bp
)

add_test(
  simple_tracepoint
  simple_tracepoint_test
)
