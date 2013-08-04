configure_file(Ice.config.in ${CMAKE_CURRENT_BINARY_DIR}/Ice.config)

add_executable(IceMiddleware_test IceMiddleware_test.cpp)
target_link_libraries(IceMiddleware_test ${LIBS} ${TESTLIBS})

add_test(IceMiddleware IceMiddleware_test)
