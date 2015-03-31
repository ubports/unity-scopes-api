configure_file(Leaf.ini.in Leaf.ini)
add_library(Leaf MODULE SHARED Leaf.cpp)
