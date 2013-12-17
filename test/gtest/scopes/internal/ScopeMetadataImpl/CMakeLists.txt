configure_file(Zmq.ini.in Zmq.ini)

add_executable(ScopeMetadataImpl_test ScopeMetadataImpl_test.cpp)
target_link_libraries(ScopeMetadataImpl_test ${TESTLIBS})

add_test(ScopeMetadataImpl ScopeMetadataImpl_test)
