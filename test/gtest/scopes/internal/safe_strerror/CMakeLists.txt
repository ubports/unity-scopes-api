add_executable(safe_strerror_test safe_strerror_test.cpp)
target_link_libraries(safe_strerror_test ${TESTLIBS})

add_test(safe_strerror safe_strerror_test)
