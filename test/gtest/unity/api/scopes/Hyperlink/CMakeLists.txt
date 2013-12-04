add_executable(Hyperlink_test Hyperlink_test.cpp)
target_link_libraries(Hyperlink_test ${TESTLIBS})

add_test(Hyperlink Hyperlink_test)
