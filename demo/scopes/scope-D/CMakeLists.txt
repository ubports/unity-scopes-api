add_library(scope-D MODULE scope-D.cpp)
add_dependencies(scope-D scoperegistry scoperunner)

configure_file(scope-D.ini.in scope-D.ini)
