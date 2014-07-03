add_definitions(-DTEST_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(RuntimeConfig_test RuntimeConfig_test.cpp)
target_link_libraries(RuntimeConfig_test ${TESTLIBS})

add_test(RuntimeConfig RuntimeConfig_test)
