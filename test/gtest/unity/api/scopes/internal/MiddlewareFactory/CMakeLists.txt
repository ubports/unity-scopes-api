configure_file(Factory.ini.in Factory.ini)
configure_file(Ice.Config.in Ice.Config)

add_executable(MiddlewareFactory_test MiddlewareFactory_test.cpp)
target_link_libraries(MiddlewareFactory_test ${TESTLIBS})

add_test(MiddlewareFactory MiddlewareFactory_test)
