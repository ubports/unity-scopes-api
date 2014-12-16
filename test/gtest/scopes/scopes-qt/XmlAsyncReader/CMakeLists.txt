add_executable(
  XmlAsyncReader_test
  XmlAsyncReader_test.cpp)

target_link_libraries(
    XmlAsyncReader_test
    ${LIBGTEST}
    ${TESTLIBS_QT})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

qt5_use_modules(XmlAsyncReader_test Core)

add_test(XmlAsyncReader XmlAsyncReader_test)