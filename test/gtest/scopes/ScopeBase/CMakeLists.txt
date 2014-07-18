add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(ScopeBase_test ScopeBase_test.cpp)
target_link_libraries(ScopeBase_test ${TESTLIBS})

add_library(scopelib SHARED scopelib.cpp)

add_test(ScopeBase ScopeBase_test)
