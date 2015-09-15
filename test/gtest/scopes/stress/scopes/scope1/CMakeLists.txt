add_library(scope1 MODULE scope1.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope1 globalheader)

configure_file(scope1.ini.in scope1.ini)
