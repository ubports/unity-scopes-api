include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
    SSRegistryObject_test

    SSRegistryObject_test.cpp
)

target_link_libraries(
    SSRegistryObject_test

    ${TESTLIBS}
)

add_test(
    SSRegistryObject

    SSRegistryObject_test
)
