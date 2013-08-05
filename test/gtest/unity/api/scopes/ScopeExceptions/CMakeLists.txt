add_executable(ScopeExceptions_test ScopeExceptions_test.cpp)
target_link_libraries(ScopeExceptions_test ${TESTLIBS})

add_test(ScopeExceptions ScopeExceptions_test)
