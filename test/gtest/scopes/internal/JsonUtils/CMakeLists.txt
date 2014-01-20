add_executable(JsonUtils_test JsonUtils_test.cpp)
target_link_libraries(JsonUtils_test ${TESTLIBS})

add_test(JsonUtils JsonUtils_test)
