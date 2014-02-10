add_executable(SearchMetadata_test SearchMetadata_test.cpp)
target_link_libraries(SearchMetadata_test ${TESTLIBS})

add_test(SearchMetadata SearchMetadata_test)
