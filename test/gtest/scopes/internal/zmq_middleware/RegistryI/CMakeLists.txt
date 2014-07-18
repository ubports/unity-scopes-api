configure_file(TestRegistry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/TestRegistry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DDEMO_DIR="${CMAKE_BINARY_DIR}/demo")
add_definitions(-DREGISTRY_TEST_DIR="${CMAKE_BINARY_DIR}/test/gtest/scopes/Registry")
add_executable(RegistryI_test RegistryI_test.cpp)
target_link_libraries(RegistryI_test ${LIBS} ${TESTLIBS})

add_test(RegistryI RegistryI_test)
