add_subdirectory(internal)
add_subdirectory(testing)
add_subdirectory(utility)
add_subdirectory(qt)

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

#
# Generated headers
#
string(REPLACE . _ UNITY_SCOPES_SOVERSION_SYM_SUFFIX ${UNITY_SCOPES_SOVERSION})
configure_file(Version.h.in Version.h)
set(headers ${headers} ${CMAKE_CURRENT_BINARY_DIR}/Version.h)

install(FILES ${headers} DESTINATION ${HDR_INSTALL_DIR}/unity/scopes)

set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} PARENT_SCOPE)
