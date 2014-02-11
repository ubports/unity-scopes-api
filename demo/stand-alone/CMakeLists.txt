configure_file(Runtime.ini.in Runtime.ini)
configure_file(Zmq.ini.in Zmq.ini)

add_executable(stand-alone-client stand-alone-client.cpp)

# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(stand-alone-client globalheader)
add_definitions(-DDEMO_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
target_link_libraries(stand-alone-client ${UNITY_SCOPES_LIB} ${OTHER_LIBS})
