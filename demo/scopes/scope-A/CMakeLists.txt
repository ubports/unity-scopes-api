add_library(scope-A MODULE scope-A.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope-A globalheader)

configure_file(scope-A.ini.in scope-A.ini)
