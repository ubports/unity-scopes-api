include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

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

file(COPY FakeSss.py DESTINATION ./)
