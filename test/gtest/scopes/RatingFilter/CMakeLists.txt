add_executable(RatingFilter_test RatingFilter_test.cpp)
target_link_libraries(RatingFilter_test ${TESTLIBS})

add_test(RatingFilter RatingFilter_test)
