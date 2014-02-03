add_executable(Util_test Util_test.cpp)
target_link_libraries(Util_test ${TESTLIBS})

add_test(Util Util_test)
