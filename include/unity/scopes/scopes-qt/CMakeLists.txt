file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

install(FILES ${headers} DESTINATION ${HDR_INSTALL_DIR}/unity/scopes/scopes-qt)
