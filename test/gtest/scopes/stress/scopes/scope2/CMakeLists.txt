configure_file(scope2.map.in scope2.map)
set(symbol_map "${CMAKE_CURRENT_BINARY_DIR}/scope2.map")

add_library(scope2 MODULE scope2.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope2 globalheader)
set_target_properties(scope2 PROPERTIES
                      LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(scope2 PROPERTIES LINK_DEPENDS ${symbol_map})

configure_file(scope2.ini.in scope2.ini)
