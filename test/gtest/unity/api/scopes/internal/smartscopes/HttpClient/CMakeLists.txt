include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(-DGOOD_SERVER_PATH="${CMAKE_SOURCE_DIR}/test/gtest/unity/api/scopes/internal/smartscopes/HttpClient/GoodServer.py")
add_definitions(-DBAD_SERVER_PATH="${CMAKE_SOURCE_DIR}/test/gtest/unity/api/scopes/internal/smartscopes/HttpClient/BadServer.py")

add_executable(
    HttpClient_test

    HttpClient_test.cpp
)

target_link_libraries(
    HttpClient_test

    ${TESTLIBS}
)

add_test(
    HttpClient

    HttpClient_test
)
