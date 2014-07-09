configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(ZmqMiddleware_test ZmqMiddleware_test.cpp)
target_link_libraries(ZmqMiddleware_test ${LIBS} ${TESTLIBS})

add_test(ZmqMiddleware ZmqMiddleware_test)
