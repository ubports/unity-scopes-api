# Set up package config.
configure_file(lib${UNITY_SCOPES_LIB}.pc.in lib${UNITY_SCOPES_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}.pc DESTINATION ${LIB_INSTALL_PREFIX}/pkgconfig)

# Set up package config.
configure_file(lib${LIB_QT}.pc.in lib${LIB_QT}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${LIB_QT}.pc DESTINATION ${LIB_INSTALL_PREFIX}/pkgconfig)

# upstart job for scope registry
configure_file(scope-registry.conf.in scope-registry.conf @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scope-registry.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/upstart/sessions)

# apport package hook
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libunity-scopes.py libunity-scopes${UNITY_SCOPES_SOVERSION}.py COPYONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libunity-scopes${UNITY_SCOPES_SOVERSION}.py
        DESTINATION ${CMAKE_INSTALL_DATADIR}/apport/package-hooks)

# Empty directory for scopes to put their conf files into
# If we start install our own conf files here, this can be
# removed (installing files implicitly creates the subdirs)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${UNITY_SCOPES_LIB})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${UNITY_SCOPES_LIB}
DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_custom_target(pkgversion ALL COMMAND dpkg-parsechangelog -l${CMAKE_SOURCE_DIR}/debian/changelog --show-field version > ${CMAKE_CURRENT_BINARY_DIR}/version)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/libunity-scopes)
