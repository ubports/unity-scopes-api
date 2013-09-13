add_executable(ServantBase_test ServantBase_test.cpp)
target_link_libraries(ServantBase_test ${LIBS} ${TESTLIBS})

add_test(ServantBase ServantBase_test)
