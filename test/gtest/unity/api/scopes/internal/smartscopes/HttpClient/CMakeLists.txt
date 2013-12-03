include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
  HttpClient_test

  HttpClient_test.cpp
)

target_link_libraries(
  HttpClient_test

  ${TESTLIBS}
)

add_test(
  HttpClient

  HttpClient_test
)
