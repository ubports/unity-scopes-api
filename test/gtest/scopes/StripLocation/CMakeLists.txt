configure_file(TestRegistry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/TestRegistry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config/Aggregator)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config/Leaf)

add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DTEST_RUNTIME_FILE="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DTEST_REGISTRY_PATH="${PROJECT_BINARY_DIR}/scoperegistry")

add_executable(StripLocation_test StripLocation_test.cpp)
target_link_libraries(StripLocation_test ${TESTLIBS})

add_dependencies(StripLocation_test scoperegistry scoperunner)

add_test(StripLocation StripLocation_test)

add_subdirectory(scopes)
