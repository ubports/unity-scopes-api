add_executable(ColumnLayout_test ColumnLayout_test.cpp)
target_link_libraries(ColumnLayout_test ${TESTLIBS})

add_test(ColumnLayout ColumnLayout_test)
