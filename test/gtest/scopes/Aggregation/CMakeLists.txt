configure_file(TestRegistry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/TestRegistry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DTEST_CACHE_PATH="${CMAKE_CURRENT_BINARY_DIR}/cache_dir")
add_definitions(-DTEST_RUNTIME_FILE="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DTEST_REGISTRY_PATH="${PROJECT_BINARY_DIR}/scoperegistry")


add_executable(ChildScopes_test ChildScopes_test.cpp)
target_link_libraries(ChildScopes_test ${TESTLIBS})

add_test(ChildScopes ChildScopes_test)


add_executable(Keywords_test Keywords_test.cpp)
target_link_libraries(Keywords_test ${TESTLIBS})

add_test(Keywords Keywords_test)


add_executable(LoopDetection_test LoopDetection_test.cpp AggTestScope.cpp)
target_link_libraries(LoopDetection_test ${TESTLIBS})

add_dependencies(LoopDetection_test scoperegistry scoperunner)

add_test(LoopDetection LoopDetection_test)

set(SCOPE_DIR "${CMAKE_CURRENT_BINARY_DIR}/scopes")

foreach (scope A B C D)
    file(MAKE_DIRECTORY "${SCOPE_DIR}/${scope}")
    configure_file(AggTestScope.ini.in ${SCOPE_DIR}/${scope}/${scope}.ini)
    add_library(${scope} MODULE AggTestScope.cpp)
    set_target_properties(${scope}
      PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${SCOPE_DIR}/${scope}/"
    )
endforeach()
