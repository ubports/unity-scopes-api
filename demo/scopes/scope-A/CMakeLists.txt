set(symbol_map "${CMAKE_CURRENT_SOURCE_DIR}/scope-A.map")

add_library(scope-A MODULE scope-A.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
# We also need the scoperegistry and scoperunner to be up-to-date.
add_dependencies(scope-A globalheader)
add_dependencies(scope-A scoperegistry scoperunner)
set_target_properties(scope-A PROPERTIES
                      LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(scope-A PROPERTIES LINK_DEPENDS ${symbol_map})

configure_file(scope-A.ini.in scope-A.ini)
