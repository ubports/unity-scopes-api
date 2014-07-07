configure_file(Test.ini.in Test.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(ConfigBase_test ConfigBase_test.cpp)
target_link_libraries(ConfigBase_test ${TESTLIBS})

add_test(ConfigBase ConfigBase_test)
