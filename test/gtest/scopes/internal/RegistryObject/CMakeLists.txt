configure_file(scope.ini.in scope.ini)

add_executable(RegistryObject_test RegistryObject_test.cpp)
target_link_libraries(RegistryObject_test ${TESTLIBS})

add_test(RegistryObject RegistryObject_test)
