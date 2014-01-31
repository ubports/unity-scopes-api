add_subdirectory(unity)
set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} PARENT_SCOPE)

add_custom_command(OUTPUT unity-scopes.h
COMMAND ${CMAKE_SOURCE_DIR}/tools/create_globalheader.py
${CMAKE_CURRENT_BINARY_DIR}/unity-scopes.h ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
DEPENDS ${UNITY_SCOPES_LIB_SRC})

add_custom_target(globalheader ALL
DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/unity-scopes.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unity-scopes.h
DESTINATION ${HDR_INSTALL_DIR})
