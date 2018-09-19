add_executable(
  JsonAsyncReader_test
  JsonAsyncReader_test.cpp)

target_link_libraries(
    JsonAsyncReader_test
    ${LIBGTEST}
    ${TESTLIBS_QT})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

target_link_libraries(JsonAsyncReader_test Qt5::Core)

add_test(JsonAsyncReader JsonAsyncReader_test)
