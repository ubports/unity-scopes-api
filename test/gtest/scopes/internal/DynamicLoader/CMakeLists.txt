add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(DynamicLoader_test DynamicLoader_test.cpp)
target_link_libraries(DynamicLoader_test ${TESTLIBS})

add_library(testlib MODULE testlib.cpp)
add_library(badtestlib MODULE badtestlib.cpp)

add_test(DynamicLoader DynamicLoader_test)
