add_executable(IniSettingsSchema_test IniSettingsSchema_test.cpp)
target_link_libraries(IniSettingsSchema_test ${TESTLIBS})

add_test(IniSettingsSchema IniSettingsSchema_test)

add_definitions(-DTEST_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
