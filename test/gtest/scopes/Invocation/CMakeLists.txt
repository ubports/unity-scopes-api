configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_executable(Invocation_test Invocation_test.cpp TestScope.cpp EmptyScope.cpp)
target_link_libraries(Invocation_test ${TESTLIBS})

add_test(Invocation Invocation_test)
