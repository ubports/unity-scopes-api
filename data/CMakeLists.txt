# Set up package config.
configure_file(lib${UNITY_SCOPES_LIB}.pc.in lib${UNITY_SCOPES_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}.pc DESTINATION ${LIB_INSTALL_PREFIX}/pkgconfig)

configure_file(Runtime.ini.in Runtime.ini @ONLY)
configure_file(Registry.ini.in Registry.ini @ONLY)
configure_file(Zmq.ini.in Zmq.ini @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini DESTINATION ${SCOPES_DEFAULT_CONFIGDIR})

install(FILES Groups.ini DESTINATION ${SCOPES_DEFAULT_DATADIR}/scope-groups)
