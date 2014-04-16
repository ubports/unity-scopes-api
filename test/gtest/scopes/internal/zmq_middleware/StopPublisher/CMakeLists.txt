add_executable(StopPublisher_test StopPublisher_test.cpp)
target_link_libraries(StopPublisher_test ${LIBS} ${TESTLIBS})

add_test(StopPublisher StopPublisher_test)
