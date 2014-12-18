add_executable(
  JsonAsyncReader_test
  JsonAsyncReader_test.cpp)

target_link_libraries(
    JsonAsyncReader_test
    ${LIBGTEST}
    ${TESTLIBS_QT})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

qt5_use_modules(JsonAsyncReader_test Core)

add_test(JsonAsyncReader JsonAsyncReader_test)