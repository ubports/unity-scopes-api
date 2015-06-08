add_executable(ActivationResponse_test ActivationResponse_test.cpp)
add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ActivationResponse_test ${TESTLIBS})

configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_test(ActivationResponse ActivationResponse_test)
