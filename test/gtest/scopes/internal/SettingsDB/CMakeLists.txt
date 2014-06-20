add_executable(SettingsDB_test SettingsDB_test.cpp)
target_link_libraries(SettingsDB_test ${TESTLIBS})

add_definitions(-DTEST_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DTEST_BIN_DIR="${CMAKE_CURRENT_BINARY_DIR}")

add_test(SettingsDB SettingsDB_test)
