add_executable(Variant_test Variant_test.cpp)
target_link_libraries(Variant_test ${TESTLIBS})

add_test(Variant Variant_test)
