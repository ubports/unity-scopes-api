add_library(scope-B MODULE scope-B.cpp)
add_dependencies(scope-B scoperegistry scoperunner)

configure_file(scope-B.ini.in scope-B.ini)
