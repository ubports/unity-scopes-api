find_package(GMock)

set(LIBGTEST gtest gmock gmock_main)
set(TESTLIBS ${TESTLIBS} ${LIBGTEST} boost_regex)

add_subdirectory(scopes)

set(TEST_SRC
)

foreach(src ${TEST_SRC})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} ${TESTLIBS})
    string(REPLACE "_test" "" test_name ${name})
    add_test(${test_name} ${name})
endforeach(src)
