if (NOT DEFINED GMOCK_ROOT)
    set(GMOCK_ROOT /usr/src/gmock)
endif()

add_subdirectory(${GMOCK_ROOT} gmock)

# Selectively switch off warnings about missing field initializers when building gmock and gtest
set_property(TARGET gmock APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-missing-field-initializers -Wno-old-style-cast")
set_property(TARGET gmock_main APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-missing-field-initializers -Wno-old-style-cast")
set_property(TARGET gtest APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-missing-field-initializers -Wno-old-style-cast")

# Make sure we always build gtest/gmock with debug info, otherwise we don't
# get line numbers for the code that is dragged in by the test macros.
set_property(TARGET gmock APPEND_STRING PROPERTY COMPILE_FLAGS " -g")
set_property(TARGET gmock_main APPEND_STRING PROPERTY COMPILE_FLAGS " -g")
set_property(TARGET gtest APPEND_STRING PROPERTY COMPILE_FLAGS " -g")

include_directories(${GMOCK_ROOT}/gtest/include)

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
