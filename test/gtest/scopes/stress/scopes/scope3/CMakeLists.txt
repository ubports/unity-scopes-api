add_library(scope3 MODULE scope3.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope3 globalheader)

configure_file(scope3.ini.in scope3.ini)
