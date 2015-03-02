configure_file(Aggregator.ini.in Aggregator.ini)
add_library(Aggregator MODULE SHARED Aggregator.cpp)
