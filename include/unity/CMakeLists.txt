add_subdirectory(api)

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

install(FILES ${headers} DESTINATION ${HDR_INSTALL_DIR}/unity)

set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} PARENT_SCOPE)
