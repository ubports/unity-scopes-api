set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/HttpAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlReader.cpp
)

# -- libunity-utils-qt --

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(
  -fno-permissive
  -std=c++11
  -pedantic
  -Wall
  -Wextra
  -fPIC
)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(
  SCOPE
  net-cpp>=1.1.0
  process-cpp
  REQUIRED
)

find_package(Qt5Core REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(
    ${CMAKE_BINARY_DIR}/include
)

add_library(
    unity-utils-qt SHARED
    ${SRC}
)

add_library(
  unity-utils-qt-static OBJECT
  ${SRC}
)

qt5_use_modules(
    unity-utils-qt

    Core
)

target_link_libraries(
    unity-utils-qt
    ${UNITY_API_LDFLAGS}
    net-cpp
)
set_target_properties(unity-utils-qt PROPERTIES
    VERSION "${UNITY_SCOPES_MAJOR}.${UNITY_SCOPES_MINOR}.${UNITY_SCOPES_MICRO}"
    SOVERSION ${UNITY_SCOPES_SOVERSION})

install(TARGETS unity-utils-qt LIBRARY DESTINATION ${LIB_INSTALL_PREFIX})
