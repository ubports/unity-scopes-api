add_executable(Reaper_test Reaper_test.cpp)
target_link_libraries(Reaper_test ${TESTLIBS})

add_test(Reaper Reaper_test)
