configure_file(Zmq.ini.in Zmq.ini)

add_executable(ObjectAdapter_test ObjectAdapter_test.cpp)
target_link_libraries(ObjectAdapter_test ${LIBS} ${TESTLIBS})

add_test(ObjectAdapter ObjectAdapter_test)
