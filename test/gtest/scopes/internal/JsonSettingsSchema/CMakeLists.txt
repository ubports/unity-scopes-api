add_executable(JsonSettingsSchema_test JsonSettingsSchema_test.cpp)
target_link_libraries(JsonSettingsSchema_test ${TESTLIBS})

add_test(JsonSettingsSchema JsonSettingsSchema_test)

add_definitions(-DTEST_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
