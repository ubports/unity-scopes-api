add_library(scope-C MODULE scope-C.cpp)
add_dependencies(scope-C scoperegistry scoperunner)

configure_file(scope-C.ini.in scope-C.ini)
