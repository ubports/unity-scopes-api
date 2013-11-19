add_subdirectory(scopes)
add_subdirectory(scope-groups)

configure_file(Factory.ini.in Factory.ini)
configure_file(Runtime.ini.in Runtime.ini)
configure_file(Registry.ini.in Registry.ini)
configure_file(Zmq.ini.in Zmq.ini)


add_executable(client client.cpp)
target_link_libraries(client ${UNITY_SCOPES_LIB} ${OTHER_LIBS})
