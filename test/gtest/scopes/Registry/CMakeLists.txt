include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(RegistryTest.ini.in ${CMAKE_CURRENT_BINARY_DIR}/RegistryTest.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_SRC_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DTEST_RUNTIME_FILE="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DTEST_REGISTRY_PATH="${PROJECT_BINARY_DIR}/scoperegistry")
add_definitions(-DTEST_SCOPERUNNER_PATH="${PROJECT_BINARY_DIR}/scoperunner")
add_definitions(-DTEST_SCOPE_A_PATH="${CMAKE_CURRENT_BINARY_DIR}/scopes/testscopeA")

add_executable(Registry_test Registry_test.cpp)
target_link_libraries(Registry_test ${TESTLIBS})

add_dependencies(Registry_test scoperegistry scoperunner)

add_test(Registry Registry_test)
add_subdirectory(scopes)
add_subdirectory(other_scopes)
