configure_file(scope3.map.in scope3.map)
set(symbol_map "${CMAKE_CURRENT_BINARY_DIR}/scope3.map")

add_library(scope3 MODULE scope3.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope3 globalheader)
set_target_properties(scope3 PROPERTIES
                      LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(scope3 PROPERTIES LINK_DEPENDS ${symbol_map})

configure_file(scope3.ini.in scope3.ini)
