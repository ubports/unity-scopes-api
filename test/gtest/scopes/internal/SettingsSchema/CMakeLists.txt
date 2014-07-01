add_executable(SettingsSchema_test SettingsSchema_test.cpp)
target_link_libraries(SettingsSchema_test ${TESTLIBS})

add_test(SettingsSchema SettingsSchema_test)

add_definitions(-DTEST_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
