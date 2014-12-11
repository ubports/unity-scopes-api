set(SCOPES_QT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/HttpAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlReader.cpp
    CACHE STRING "Source files using Qt" FORCE
)

# add -fPIC specifically to Qt content files
# we need the flag in order to compile with Qt
#set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS -fPIC)

# -- libunity-scopes-qt --

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
    unity-scopes-qt SHARED
    ${SCOPES_QT_SRC}
)

qt5_use_modules(
    unity-scopes-qt

    Core
)

set(LIBS unity-scopes-qt)

target_link_libraries(
    unity-scopes-qt
    ${UNITY_API_LDFLAGS}
    net-cpp
)

# API version
set(UNITY_SCOPES_QT_MAJOR 0)
set(UNITY_SCOPES_QT_MINOR 1)
set(UNITY_SCOPES_QT_MICRO 0)
set(UNITY_SCOPES_QT_SOVERSION ${UNITY_SCOPES_QT_MAJOR}${UNITY_SCOPES_QT_MINOR})

set_target_properties(unity-scopes-qt PROPERTIES
    VERSION "${UNITY_SCOPES_QT_MAJOR}.${UNITY_SCOPES_QT_MINOR}.${UNITY_SCOPES_QT_MICRO}"
    SOVERSION ${UNITY_SCOPES_QT_SOVERSION})

#set(LIB_INSTALL_PREFIX lib/${CMAKE_LIBRARY_ARCHITECTURE})
install(TARGETS unity-scopes-qt LIBRARY DESTINATION ${LIB_INSTALL_PREFIX})


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
    configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile-scopes-qt.in ${PROJECT_BINARY_DIR}/doc/Doxyfile-scopes-qt @ONLY IMMEDIATE)
#    configure_file(${PROJECT_SOURCE_DIR}/doc/index.html ${PROJECT_BINARY_DIR}/doc/index.html COPYONLY)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/doc-scopes-qt/index.html
                       COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile-scopes-qt
                       DEPENDS ${PROJECT_BINARY_DIR}/doc/Doxyfile-scopes-qt
                               ${CMAKE_BINARY_DIR}/include/unity/scopes/scopes-qt)
    add_custom_target(doc-scopes-qt ALL DEPENDS ${PROJECT_BINARY_DIR}/doc-scopes-qt/index.html)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc-scopes-qt/unity-scopes-qt
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc)
endif()
