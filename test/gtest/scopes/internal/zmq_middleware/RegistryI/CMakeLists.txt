configure_file(TestRegistry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/TestRegistry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_executable(RegistryI_test RegistryI_test.cpp)
target_link_libraries(RegistryI_test ${LIBS} ${TESTLIBS})

add_test(RegistryI RegistryI_test)
