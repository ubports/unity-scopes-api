add_executable(Query_test Query_test.cpp)
target_link_libraries(Query_test ${TESTLIBS})

add_test(Query Query_test)
