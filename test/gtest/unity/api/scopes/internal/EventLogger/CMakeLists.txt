add_executable(EventLogger_test EventLogger_test.cpp)
target_link_libraries(EventLogger_test ${TESTLIBS})

add_test(EventLogger EventLogger_test)
