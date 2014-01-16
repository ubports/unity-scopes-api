include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
    SSScopeObject_test

    SSScopeObject_test.cpp
)

target_link_libraries(
    SSScopeObject_test

    ${TESTLIBS}
)

add_test(
    SSScopeObject

    SSScopeObject_test
)
