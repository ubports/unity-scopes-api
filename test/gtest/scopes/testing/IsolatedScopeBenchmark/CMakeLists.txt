# Please adjust this value with extreme care.
# It is common sense to keep the confidence level at 95%.
set(
  BENCHMARKING_TEST_CONFIDENCE_LEVEL 0.95
  CACHE STRING "Confidence level for statistical evaluation. The higher, the more likely the test will pass")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    @ONLY
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(IsolatedScopeBenchmark_test IsolatedScopeBenchmark_test.cpp scope.cpp)
target_link_libraries(IsolatedScopeBenchmark_test ${TESTLIBS})

add_test(IsolatedScopeBenchmark IsolatedScopeBenchmark_test)
