configure_file(Test.ini.in Test.ini)

add_executable(ConfigBase_test ConfigBase_test.cpp)
target_link_libraries(ConfigBase_test ${TESTLIBS})

add_test(ConfigBase ConfigBase_test)
