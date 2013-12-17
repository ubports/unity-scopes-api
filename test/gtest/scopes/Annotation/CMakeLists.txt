add_executable(Annotation_test Annotation_test.cpp)
target_link_libraries(Annotation_test ${TESTLIBS})

add_test(Annotation Annotation_test)
