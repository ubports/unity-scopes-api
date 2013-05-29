add_executable(DynamicLoader_test DynamicLoader_test.cpp)
target_link_libraries(DynamicLoader_test ${LIBS} ${TESTLIBS})

add_library(testlib SHARED testlib.cpp)
add_library(badtestlib SHARED badtestlib.cpp)

add_test(DynamicLoader DynamicLoader_test)
