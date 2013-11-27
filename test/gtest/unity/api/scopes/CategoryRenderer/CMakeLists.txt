configure_file(renderer.json ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
add_executable(CategoryRenderer_test CategoryRenderer_test.cpp)
target_link_libraries(CategoryRenderer_test ${TESTLIBS})

add_test(CategoryRenderer CategoryRenderer_test)
