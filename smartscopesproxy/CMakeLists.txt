set(SRC smartscopesproxy.cpp )

add_executable(smartscopesproxy ${SRC})
target_link_libraries(smartscopesproxy ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

install(TARGETS smartscopesproxy RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/smartscopesproxy)
