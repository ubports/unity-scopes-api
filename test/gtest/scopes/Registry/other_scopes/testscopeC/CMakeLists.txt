add_library(testscopeC MODULE testscopeC.cpp)
target_link_libraries(testscopeC ${LIBGTEST})
configure_file(testscopeC.ini.in testscopeC.ini)
