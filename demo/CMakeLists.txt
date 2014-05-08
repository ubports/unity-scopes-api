add_subdirectory(click)
add_subdirectory(scopes)
add_subdirectory(stand-alone)

configure_file(Runtime.ini.in Runtime.ini)
configure_file(Registry.ini.in Registry.ini)

add_definitions(-DDEMO_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")

add_executable(scopes-client client.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scopes-client globalheader)
target_link_libraries(scopes-client ${UNITY_SCOPES_LIB} ${OTHER_LIBS})
install(TARGETS scopes-client DESTINATION bin)
