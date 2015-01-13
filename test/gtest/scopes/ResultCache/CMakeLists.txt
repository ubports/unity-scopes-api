configure_file(TestRegistry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/TestRegistry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DTEST_RUNTIME_FILE="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DTEST_REGISTRY_PATH="${PROJECT_BINARY_DIR}/scoperegistry")

add_executable(ResultCache_test ResultCache_test.cpp CacheScope.cpp)
target_link_libraries(ResultCache_test ${TESTLIBS})

add_dependencies(ResultCache_test scoperegistry scoperunner)

add_test(ResultCache ResultCache_test)

set(SCOPE_DIR "${CMAKE_CURRENT_BINARY_DIR}/scopes")

foreach (scope CacheScope AlwaysPushFromCacheScope)
    file(MAKE_DIRECTORY "${SCOPE_DIR}/${scope}")
    configure_file(CacheScope.ini.in ${SCOPE_DIR}/${scope}/${scope}.ini)
    add_library(${scope} MODULE CacheScope.cpp)
    set_target_properties(${scope}
      PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${SCOPE_DIR}/${scope}/"
    )
endforeach()
