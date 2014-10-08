add_executable(OnlineAccountClient_test OnlineAccountClient_test.cpp)

pkg_check_modules(DBUSTEST REQUIRED dbustest-1)
include_directories(${DBUSTEST_INCLUDE_DIRS})
target_link_libraries(OnlineAccountClient_test ${DBUSTEST_LDFLAGS})

add_definitions(-DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_definitions(-DTEST_DB_DIR="${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(OnlineAccountClient_test ${TESTLIBS})

add_test(OnlineAccountClient OnlineAccountClient_test)
