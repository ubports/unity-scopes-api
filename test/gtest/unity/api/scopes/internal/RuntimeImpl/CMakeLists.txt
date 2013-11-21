configure_file(Factory.ini.in Factory.ini)
configure_file(Zmq.ini.in Zmq.ini)
configure_file(Registry.ini.in Registry.ini)
configure_file(Runtime.ini.in Runtime.ini)

add_executable(RuntimeImpl_test RuntimeImpl_test.cpp)
target_link_libraries(RuntimeImpl_test ${TESTLIBS})

add_test(RuntimeImpl RuntimeImpl_test)
