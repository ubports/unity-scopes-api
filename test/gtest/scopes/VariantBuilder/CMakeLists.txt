add_executable(VariantBuilder_test VariantBuilder_test.cpp)
target_link_libraries(VariantBuilder_test ${TESTLIBS})

add_test(VariantBuilder VariantBuilder_test)
