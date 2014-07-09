configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(ServantBase_test ServantBase_test.cpp)
target_link_libraries(ServantBase_test ${LIBS} ${TESTLIBS})

add_test(ServantBase ServantBase_test)
