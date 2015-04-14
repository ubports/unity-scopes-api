add_executable(Utils_test Utils_test.cpp)
add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(Utils_test ${TESTLIBS})

add_test(Utils Utils_test)
