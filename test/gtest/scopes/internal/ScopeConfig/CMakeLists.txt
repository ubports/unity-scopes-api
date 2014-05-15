configure_file(configtest1.ini.in configtest1.ini copyonly)
configure_file(configtest2.ini.in configtest2.ini copyonly)
configure_file(configtest3.ini.in configtest3.ini copyonly)

add_definitions(-DTEST_CONFIG_1="${CMAKE_CURRENT_BINARY_DIR}/configtest1.ini")
add_definitions(-DTEST_CONFIG_2="${CMAKE_CURRENT_BINARY_DIR}/configtest2.ini")
add_definitions(-DTEST_CONFIG_3="${CMAKE_CURRENT_BINARY_DIR}/configtest3.ini")

add_executable(ScopeConfig_test ScopeConfig_test.cpp)
target_link_libraries(ScopeConfig_test ${TESTLIBS})

add_test(ScopeConfig ScopeConfig_test)
