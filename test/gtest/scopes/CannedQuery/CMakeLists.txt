add_executable(CannedQuery_test CannedQuery_test.cpp)
target_link_libraries(CannedQuery_test ${TESTLIBS})

add_test(CannedQuery CannedQuery_test)
