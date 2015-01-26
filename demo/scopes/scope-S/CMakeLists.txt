add_library(scope-S MODULE scope-S.cpp)
add_dependencies(scope-S scoperegistry scoperunner)

configure_file(scope-S.ini.in scope-S.ini)
