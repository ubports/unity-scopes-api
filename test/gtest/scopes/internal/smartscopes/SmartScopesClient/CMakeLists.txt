include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(partnerid ${CMAKE_CURRENT_BINARY_DIR}/partnerid COPYONLY)

add_definitions(-DFAKE_SSS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/FakeSss.py")
add_definitions(-DFAKE_SSS_LOG="${CMAKE_CURRENT_BINARY_DIR}/fakesss.log")
add_definitions(-DPARTNER_FILE="${CMAKE_CURRENT_BINARY_DIR}/partnerid")

add_executable(
    SmartScopesClient_test

    SmartScopesClient_test.cpp
)

target_link_libraries(
    SmartScopesClient_test

    ${TESTLIBS}
)

add_test(
    SmartScopesClient

    SmartScopesClient_test
)
