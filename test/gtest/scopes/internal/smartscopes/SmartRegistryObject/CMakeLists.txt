include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
    SmartRegistryObject_test

    SmartRegistryObject_test.cpp
)

target_link_libraries(
    SmartRegistryObject_test

    ${TESTLIBS}
)

add_test(
    SmartRegistryObject

    SmartRegistryObject_test
)
