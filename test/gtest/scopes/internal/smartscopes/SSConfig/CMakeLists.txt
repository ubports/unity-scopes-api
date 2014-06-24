configure_file(SSRegistry.ini.in SSRegistry.ini)

add_definitions(-DTEST_SSREGISTRY_PATH="${CMAKE_CURRENT_BINARY_DIR}/SSRegistry.ini")

add_executable(SSConfig_test SSConfig_test.cpp)
target_link_libraries(SSConfig_test ${TESTLIBS})

add_test(SSConfig SSConfig_test)
