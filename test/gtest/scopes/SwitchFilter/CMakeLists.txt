add_executable(SwitchFilter_test SwitchFilter_test.cpp)
target_link_libraries(SwitchFilter_test ${TESTLIBS})

add_test(SwitchFilter SwitchFilter_test)
