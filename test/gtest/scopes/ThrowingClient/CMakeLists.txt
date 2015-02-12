configure_file(TestRegistry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/TestRegistry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DTEST_RUNTIME_FILE="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DTEST_REGISTRY_PATH="${PROJECT_BINARY_DIR}/scoperegistry")

add_executable(ThrowingClient_test ThrowingClient_test.cpp)
target_link_libraries(ThrowingClient_test ${TESTLIBS})

add_dependencies(ThrowingClient_test scoperegistry scoperunner)

add_test(ThrowingClient ThrowingClient_test)

set(SCOPE_DIR "${CMAKE_CURRENT_BINARY_DIR}/scopes")

foreach (scope EchoScope)
    file(MAKE_DIRECTORY "${SCOPE_DIR}/${scope}")
    configure_file(EchoScope.ini.in ${SCOPE_DIR}/${scope}/${scope}.ini)
    add_library(${scope} MODULE EchoScope.cpp)
    set_target_properties(${scope}
      PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${SCOPE_DIR}/${scope}/"
    )
endforeach()
