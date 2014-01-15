include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
    SmartScopeObject_test

    SmartScopeObject_test.cpp
)

target_link_libraries(
    SmartScopeObject_test

    ${TESTLIBS}
)

add_test(
    SmartScopeObject

    SmartScopeObject_test
)
