file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

install(FILES ${headers} DESTINATION ${HDR_INSTALL_DIR}/unity/scopes/testing)

set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} PARENT_SCOPE)
