configure_file(Runtime.ini.in Runtime.ini)
configure_file(Registry.ini.in Registry.ini)
configure_file(Zmq.ini.in Zmq.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(ObjectAdapter_test ObjectAdapter_test.cpp)
target_link_libraries(ObjectAdapter_test ${LIBS} ${TESTLIBS})

add_test(ObjectAdapter ObjectAdapter_test)
