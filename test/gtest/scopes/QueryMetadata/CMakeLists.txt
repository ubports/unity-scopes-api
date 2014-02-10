add_executable(QueryMetadata_test QueryMetadata_test.cpp)
target_link_libraries(QueryMetadata_test ${TESTLIBS})

add_test(QueryMetadata QueryMetadata_test)
