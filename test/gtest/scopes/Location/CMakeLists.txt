add_executable(Location_test Location_test.cpp)
target_link_libraries(Location_test ${TESTLIBS})

add_test(Location Location_test)
