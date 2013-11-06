add_executable(ResultItem_test ResultItem_test.cpp)
target_link_libraries(ResultItem_test ${TESTLIBS})

add_test(ResultItem ResultItem_test)