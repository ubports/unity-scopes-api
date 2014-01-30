add_executable(PreviewWidget_test PreviewWidget_test.cpp)
target_link_libraries(PreviewWidget_test ${TESTLIBS})

add_test(PreviewWidget PreviewWidget_test)
