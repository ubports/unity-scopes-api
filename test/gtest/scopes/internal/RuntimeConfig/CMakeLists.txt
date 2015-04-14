add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(RuntimeConfig_test RuntimeConfig_test.cpp)
target_link_libraries(RuntimeConfig_test ${TESTLIBS})

file(GLOB ini_files "*.ini.in")
foreach(ini_in ${ini_files})
    get_filename_component(file ${ini_in} NAME)
    string(REGEX REPLACE "(.*)\\.in$" "\\1" ini ${file})
    configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/${ini})
endforeach(ini_in)

add_test(RuntimeConfig RuntimeConfig_test)
