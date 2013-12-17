configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_executable(Runtime_test Runtime_test.cpp)
target_link_libraries(Runtime_test ${TESTLIBS})

add_test(Runtime Runtime_test)

add_executable(Runtime_TestScope scope.cpp)
target_link_libraries(Runtime_TestScope ${TESTLIBS})
