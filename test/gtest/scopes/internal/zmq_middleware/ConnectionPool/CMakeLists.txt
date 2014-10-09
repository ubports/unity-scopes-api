add_executable(ConnectionPool_test ConnectionPool_test.cpp)
target_link_libraries(ConnectionPool_test ${LIBS} ${TESTLIBS})

add_test(ConnectionPool ConnectionPool_test)
