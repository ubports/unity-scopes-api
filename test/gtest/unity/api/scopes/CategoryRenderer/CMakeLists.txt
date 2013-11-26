configure_file(renderer.json.in ${CMAKE_CURRENT_BINARY_DIR}/renderer.json)
add_executable(CategoryRenderer_test CategoryRenderer_test.cpp)
target_link_libraries(CategoryRenderer_test ${TESTLIBS})

add_test(CategoryRenderer CategoryRenderer_test)
