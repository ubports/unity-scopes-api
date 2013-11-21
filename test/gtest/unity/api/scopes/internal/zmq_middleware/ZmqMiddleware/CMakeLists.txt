configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_executable(ZmqMiddleware_test ZmqMiddleware_test.cpp)
target_link_libraries(ZmqMiddleware_test ${LIBS} ${TESTLIBS})

add_test(ZmqMiddleware ZmqMiddleware_test)
