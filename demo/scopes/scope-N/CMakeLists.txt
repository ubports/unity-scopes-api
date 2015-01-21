add_library(scope-N MODULE scope-N.cpp)
add_dependencies(scope-N scoperegistry scoperunner)

configure_file(scope-N.ini.in scope-N.ini)
