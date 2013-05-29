# This allows us to use a shared library in the current dir for testing without having to set LD_LIBRARY_PATH
add_definitions(-DTESTLIBDIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

add_executable(ScopeBase_test ScopeBase_test.cpp)
target_link_libraries(ScopeBase_test ${LIBS} ${TESTLIBS})

add_library(scopelib SHARED scopelib.cpp)

add_test(ScopeBase ScopeBase_test)
