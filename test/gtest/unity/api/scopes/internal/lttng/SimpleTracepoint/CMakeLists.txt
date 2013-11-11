include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
  SimpleTracepoint_test

  SimpleTracepoint_test.cpp
  TestProvider_tp.cpp
  TestProvider2_tp.cpp
)

add_library(
  testtracer SHARED

  TestProvider_tp.c
  TestProvider2_tp.c
)

add_definitions(-DTRACEPOINT_LIB="${CMAKE_CURRENT_BINARY_DIR}/libtesttracer.so")

target_link_libraries(
  SimpleTracepoint_test

  ${TESTLIBS}
)

target_link_libraries(
  testtracer

  lttng-ust
  urcu-bp
  dl
)

add_test(
  SimpleTracepoint

  SimpleTracepoint_test
)
