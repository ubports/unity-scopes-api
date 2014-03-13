configure_file(configtest1.ini.in configtest1.ini copyonly)
configure_file(configtest2.ini.in configtest2.ini copyonly)

add_executable(ScopeConfig_test ScopeConfig_test.cpp)
target_link_libraries(ScopeConfig_test ${TESTLIBS})

add_test(ScopeConfig ScopeConfig_test)
