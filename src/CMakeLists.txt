add_subdirectory(unity)
set(SLICE_FILES ${SLICE_FILES} PARENT_SCOPE)

# Set up package config.
configure_file(lib${UNITY_SCOPES_LIB}.pc.in lib${UNITY_SCOPES_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}.pc DESTINATION ${LIB_INSTALL_PREFIX}/pkgconfig)

set (SRC
)

# Parent needs to know what all the source files are, for generating doc and the like.
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
