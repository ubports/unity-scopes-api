configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)
configure_file(DebugScope.ini.in ${CMAKE_CURRENT_BINARY_DIR}/DebugScope.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")

add_executable(ReplyReaper_test ReplyReaper_test.cpp NoReplyScope.cpp)
target_link_libraries(ReplyReaper_test ${TESTLIBS})

add_test(ReplyReaper ReplyReaper_test)
