include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
  SimpleTracepoint_test

  SimpleTracepoint_test.cpp
  TestProvider_tp.cpp
)

add_library(
  testtracer

  TestProvider_tp.c
)

target_link_libraries(
  SimpleTracepoint_test

  ${TESTLIBS}
  testtracer
)

target_link_libraries(
  testtracer

  lttng-ust
  urcu-bp
)

add_test(
  SimpleTracepoint

  SimpleTracepoint_test
)
