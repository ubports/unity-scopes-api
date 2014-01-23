add_subdirectory(unity)
set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} PARENT_SCOPE)

add_custom_target(globalheader ALL
COMMAND ${CMAKE_SOURCE_DIR}/tools/create_globalheader.py ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_BINARY_DIR}/donotincludeme.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/donotincludeme.h
DESTINATION ${HDR_INSTALL_DIR} RENAME unity-scopes.h)
