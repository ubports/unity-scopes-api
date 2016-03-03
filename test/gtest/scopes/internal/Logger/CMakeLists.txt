add_executable(Logger_test Logger_test.cpp)
target_link_libraries(Logger_test ${TESTLIBS})

add_test(Logger Logger_test)
