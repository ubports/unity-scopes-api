include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
  JsonParser_test

  JsonParser_test.cpp
)

target_link_libraries(
  JsonParser_test

  ${TESTLIBS}
)

add_test(
  JsonParser

  JsonParser_test
)
