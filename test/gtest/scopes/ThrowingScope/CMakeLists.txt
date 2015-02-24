configure_file(TestRegistry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/TestRegistry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DTEST_RUNTIME_FILE="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DTEST_REGISTRY_PATH="${PROJECT_BINARY_DIR}/scoperegistry")

add_executable(ThrowingScope_test ThrowingScope_test.cpp)
target_link_libraries(ThrowingScope_test ${TESTLIBS})

add_dependencies(ThrowingScope_test scoperegistry scoperunner)

add_test(ThrowingScope ThrowingScope_test)

set(SCOPE_DIR "${CMAKE_CURRENT_BINARY_DIR}/scopes")

foreach (scope ThrowingScope)
    file(MAKE_DIRECTORY "${SCOPE_DIR}/${scope}")
    configure_file(ThrowingScope.ini.in ${SCOPE_DIR}/${scope}/${scope}.ini)
    add_library(${scope} MODULE ThrowingScope.cpp)
    set_target_properties(${scope}
      PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${SCOPE_DIR}/${scope}/"
    )
endforeach()
