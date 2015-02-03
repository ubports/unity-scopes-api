configure_file(Runtime.ini.in Runtime.ini)

add_executable(stand-alone-client stand-alone-client.cpp)

# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary, and
# we need an up-to-date registry to run it.
add_dependencies(stand-alone-client globalheader)
add_dependencies(stand-alone-client scoperegistry)
add_definitions(-DDEMO_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
target_link_libraries(stand-alone-client ${UNITY_SCOPES_LIB} ${OTHER_LIBS})
