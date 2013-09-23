add_executable(UniqueID_test UniqueID_test.cpp)
target_link_libraries(UniqueID_test ${TESTLIBS})

add_test(UniqueID UniqueID_test)
