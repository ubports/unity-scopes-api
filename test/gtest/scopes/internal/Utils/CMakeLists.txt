add_executable(Utils_test Utils_test.cpp)
target_link_libraries(Utils_test ${TESTLIBS})

add_test(Utils Utils_test)
