set(
  UNITY_SCOPES_API_BENCHMARKING_TEST_CONFIDENCE_LEVEL 0.5
  CACHE STRING "Confidence level for statistical evaluation. The higher, the more likely the test will pass")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    @ONLY
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(Statistics_test Statistics_test.cpp)
target_link_libraries(Statistics_test ${TESTLIBS})

add_test(Statistics Statistics_test)
