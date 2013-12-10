include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(-DFAKE_SSS_PATH="${CMAKE_SOURCE_DIR}/test/gtest/unity/api/scopes/internal/smartscopes/SmartScopesClient/FakeSss.py")

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
