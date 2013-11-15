add_executable(ThreadPool_test ThreadPool_test.cpp)
target_link_libraries(ThreadPool_test ${TESTLIBS})

add_test(ThreadPool ThreadPool_test)
