add_subdirectory(scopes)

configure_file(Runtime.ini.in Runtime.ini)
configure_file(Registry.ini.in Registry.ini)
configure_file(Zmq.ini.in Zmq.ini)

add_definitions(-DDEMO_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")

add_executable(client client.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(client globalheader)
target_link_libraries(client ${UNITY_SCOPES_LIB} ${OTHER_LIBS})
