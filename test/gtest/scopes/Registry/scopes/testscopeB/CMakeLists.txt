add_executable(testscopeB testscopeB.cpp)
target_link_libraries(testscopeB ${UNITY_SCOPES_LIB})
set_target_properties(testscopeB
  PROPERTIES
    PREFIX "${DEB_HOST_MULTIARCH}/"
)
configure_file(testscopeB.ini.in testscopeB.ini)
