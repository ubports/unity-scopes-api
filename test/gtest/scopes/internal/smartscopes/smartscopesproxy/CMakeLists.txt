configure_file(SSRegistryTest.ini.in ${CMAKE_CURRENT_BINARY_DIR}/SSRegistryTest.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DSS_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DFAKE_SSS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../SmartScopesClient/FakeSss.py")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
    smartscopesproxy_test

    smartscopesproxy_test.cpp
)

target_link_libraries(
    smartscopesproxy_test

    ${TESTLIBS}
)

add_test(
    smartscopesproxy

    smartscopesproxy_test
)
