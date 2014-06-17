add_subdirectory(internal)
add_subdirectory(testing)

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

#
# Generated headers
#
configure_file(Version.h.in Version.h)
set(headers ${headers} ${CMAKE_CURRENT_BINARY_DIR}/Version.h)

configure_file(ScopeBase.h.in ScopeBase.h)
set(headers ${headers} ${CMAKE_CURRENT_BINARY_DIR}/ScopeBase.h)

install(FILES ${headers} DESTINATION ${HDR_INSTALL_DIR}/unity/scopes)

set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} PARENT_SCOPE)
