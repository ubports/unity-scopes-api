# This allows us to use a shared library in the current dir for testing without having to set LD_LIBRARY_PATH
add_definitions(-DTESTLIBDIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

add_executable(DynamicLoader_test DynamicLoader_test.cpp)
target_link_libraries(DynamicLoader_test ${LIBS} ${TESTLIBS})

add_library(testlib SHARED testlib.cpp)
add_library(badtestlib SHARED badtestlib.cpp)

add_test(DynamicLoader DynamicLoader_test)
