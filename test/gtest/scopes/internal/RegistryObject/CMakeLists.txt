add_executable(RegistryObject_test RegistryObject_test.cpp)
target_link_libraries(RegistryObject_test ${TESTLIBS})

add_test(RegistryObject RegistryObject_test)
