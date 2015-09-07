add_library(scope2 MODULE scope2.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope2 globalheader)

configure_file(scope2.ini.in scope2.ini)
