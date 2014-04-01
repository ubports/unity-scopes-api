add_executable(testscopeB testscopeB.cpp)
target_link_libraries(testscopeB ${UNITY_SCOPES_LIB})
configure_file(testscopeB.ini.in testscopeB.ini)
