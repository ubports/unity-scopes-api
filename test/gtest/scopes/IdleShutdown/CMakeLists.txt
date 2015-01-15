configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(SlowSearchScope.ini.in ${CMAKE_CURRENT_BINARY_DIR}/SlowSearchScope.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_executable(IdleShutdown_test IdleShutdown_test.cpp SlowSearchScope.cpp)
target_link_libraries(IdleShutdown_test ${TESTLIBS})

add_test(IdleShutdown IdleShutdown_test)
