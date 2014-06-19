add_executable(SettingsDB_test SettingsDB_test.cpp)
target_link_libraries(SettingsDB_test ${TESTLIBS})

add_test(SettingsDB SettingsDB_test)
