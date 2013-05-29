add_subdirectory(scopes)

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB internal_headers "${CMAKE_CURRENT_SOURCE_DIR}/internal/*.h")

set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} ${internal_headers} PARENT_SCOPE)

install(FILES ${headers} DESTINATION ${HDR_INSTALL_DIR}/unity/api/scopes)
