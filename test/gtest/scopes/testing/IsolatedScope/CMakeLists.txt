add_executable(IsolatedScope_test IsolatedScope_test.cpp scope.cpp)
target_link_libraries(IsolatedScope_test ${TESTLIBS})

add_test(IsolatedScope IsolatedScope_test)
