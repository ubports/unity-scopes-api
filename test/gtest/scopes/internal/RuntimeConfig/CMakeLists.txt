configure_file(BadMW.ini.in BadMW.ini)
configure_file(BadRegID.ini.in BadRegID.ini)
configure_file(MWEmpty.ini.in MWEmpty.ini)
configure_file(RegFileEmpty.ini.in RegFileEmpty.ini)
configure_file(RegIDEmpty.ini.in RegIDEmpty.ini)
configure_file(Runtime.ini.in Runtime.ini)

add_executable(RuntimeConfig_test RuntimeConfig_test.cpp)
target_link_libraries(RuntimeConfig_test ${TESTLIBS})

add_test(RuntimeConfig RuntimeConfig_test)
