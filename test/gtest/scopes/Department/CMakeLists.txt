add_executable(Department_test Department_test.cpp)
target_link_libraries(Department_test ${TESTLIBS})

add_test(Department Department_test)
