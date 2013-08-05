configure_file(Factory.ini.in Factory.ini)
configure_file(Ice.Config.in Ice.Config)
configure_file(Registry.ini.in Registry.ini)
configure_file(Scopes.ini.in Scopes.ini)

add_executable(Runtime_test Runtime_test.cpp)
target_link_libraries(Runtime_test ${TESTLIBS})

add_test(Runtime Runtime_test)
