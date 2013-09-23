add_executable(ThreadSafeQueue_test ThreadSafeQueue_test.cpp)
target_link_libraries(ThreadSafeQueue_test ${TESTLIBS})

add_test(ThreadSafeQueue ThreadSafeQueue_test)
