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

set(LIBS unity-utils-qt)

target_link_libraries(
    unity-utils-qt
    ${UNITY_API_LDFLAGS}
    net-cpp
)
set_target_properties(unity-utils-qt PROPERTIES
    VERSION "${UNITY_SCOPES_MAJOR}.${UNITY_SCOPES_MINOR}.${UNITY_SCOPES_MICRO}"
    SOVERSION ${UNITY_SCOPES_SOVERSION})

set(LIB_INSTALL_PREFIX lib/${CMAKE_LIBRARY_ARCHITECTURE})
install(TARGETS unity-utils-qt LIBRARY DESTINATION ${LIB_INSTALL_PREFIX})


#
# Documentation
#
# Pass -DDEVEL_DOCS=ON to cmake for more detailed documentation

option(DEVEL_DOCS "Enable detailed Doxygen documentation")

find_package(Doxygen)
find_program(DOT_EXECUTABLE dot /usr/bin)
if (NOT DOXYGEN_FOUND OR NOT DOT_EXECUTABLE)
    message(WARNING "Cannot generate documentation: doxygen and/or graphviz not found")
else()
    configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile-utils-qt.in ${PROJECT_BINARY_DIR}/doc/Doxyfile-utils-qt @ONLY IMMEDIATE)
#    configure_file(${PROJECT_SOURCE_DIR}/doc/index.html ${PROJECT_BINARY_DIR}/doc/index.html COPYONLY)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/doc-utils-qt/index.html
                       COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile-utils-qt
                       DEPENDS ${PROJECT_BINARY_DIR}/doc/Doxyfile-utils-qt
                               ${CMAKE_BINARY_DIR}/include/unity/scopes/utils-qt)
    add_custom_target(doc-utils-qt ALL DEPENDS ${PROJECT_BINARY_DIR}/doc-utils-qt/index.html)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc-utils-qt/unity-utils-qt
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc)
endif()
