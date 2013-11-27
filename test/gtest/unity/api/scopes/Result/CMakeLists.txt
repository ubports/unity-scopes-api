add_executable(Result_test Result_test.cpp)
target_link_libraries(Result_test ${TESTLIBS})

add_test(ResultItem Result_test)
