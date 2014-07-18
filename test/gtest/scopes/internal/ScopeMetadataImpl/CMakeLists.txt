configure_file(Runtime.ini.in Runtime.ini)
configure_file(Registry.ini.in Registry.ini)
configure_file(Zmq.ini.in Zmq.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(ScopeMetadataImpl_test ScopeMetadataImpl_test.cpp)
target_link_libraries(ScopeMetadataImpl_test ${TESTLIBS})

add_test(ScopeMetadataImpl ScopeMetadataImpl_test)
