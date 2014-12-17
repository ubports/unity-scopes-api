include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_BIN_DIR="${CMAKE_CURRENT_BINARY_DIR}")

add_executable(ChildScopes_test ChildScopes_test.cpp)
target_link_libraries(ChildScopes_test ${TESTLIBS})

add_test(ChildScopes ChildScopes_test)
