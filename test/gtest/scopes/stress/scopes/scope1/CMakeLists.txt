configure_file(scope1.map.in scope1.map)
set(symbol_map "${CMAKE_CURRENT_BINARY_DIR}/scope1.map")

add_library(scope1 MODULE scope1.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope1 globalheader)
set_target_properties(scope1 PROPERTIES
                      LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(scope1 PROPERTIES LINK_DEPENDS ${symbol_map})

configure_file(scope1.ini.in scope1.ini)
