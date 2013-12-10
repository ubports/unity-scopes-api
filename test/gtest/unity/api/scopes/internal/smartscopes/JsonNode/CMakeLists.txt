include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
    JsonNode_test

    JsonNode_test.cpp
)

target_link_libraries(
    JsonNode_test

    ${TESTLIBS}
)

add_test(
    JsonNode

    JsonNode_test
)
