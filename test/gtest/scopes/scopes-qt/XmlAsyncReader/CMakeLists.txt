add_executable(
  XmlAsyncReader_test
  XmlAsyncReader_test.cpp
  ${SCOPES_QT_SRC})

target_link_libraries(
  XmlAsyncReader_test
  ${TESTLIBS})

target_link_libraries(
  XmlAsyncReader_test
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
  ${SCOPE_LDFLAGS}
)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

qt5_use_modules(XmlAsyncReader_test Core)

add_test(XmlAsyncReader XmlAsyncReader_test)