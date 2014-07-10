add_executable(scopetest ${CMAKE_SOURCE_DIR}/debian/tests/scopetest.cpp)
target_link_libraries(scopetest ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

add_test(autopkg-scopetest scopetest)
