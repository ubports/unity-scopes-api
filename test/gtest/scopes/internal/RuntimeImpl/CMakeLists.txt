configure_file(Registry.ini.in Registry.ini)
configure_file(Runtime.ini.in Runtime.ini)
configure_file(TestScope.ini.in TestScope.ini)
configure_file(TestScope_TestScope.ini.in TestScope_TestScope.ini)
configure_file(Zmq.ini.in Zmq.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(RuntimeImpl_test RuntimeImpl_test.cpp TestScope.cpp)
target_link_libraries(RuntimeImpl_test ${TESTLIBS})

add_test(RuntimeImpl RuntimeImpl_test)
