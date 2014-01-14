# Set up package config.
configure_file(lib${UNITY_SCOPES_LIB}.pc.in lib${UNITY_SCOPES_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}.pc DESTINATION ${LIB_INSTALL_PREFIX}/pkgconfig)

# scope framework config files
configure_file(Runtime.ini.in Runtime.ini @ONLY)
configure_file(Registry.ini.in Registry.ini @ONLY)
configure_file(Zmq.ini.in Zmq.ini @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini DESTINATION ${SCOPES_DEFAULT_CONFIGDIR})

# scope groups directory should be present
install(FILES Groups.ini DESTINATION ${SCOPES_DEFAULT_DATADIR}/scope-groups)

# upstart job for scope registry
configure_file(scope-registry.conf.in scope-registry.conf @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scope-registry.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/upstart/sessions)

# Empty directory for scopes to put their conf files into.
# If we start install our own conf files here, this can be
# removed (installing files implicitly creates the subdirs)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scopes)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scopes
DESTINATION ${SCOPES_DEFAULT_DATADIR})
