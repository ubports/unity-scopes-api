add_executable(Category_test Category_test.cpp)
target_link_libraries(Category_test ${TESTLIBS})

add_test(Category Category_test)
