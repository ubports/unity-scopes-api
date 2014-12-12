set(SCOPES_QT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/HttpAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlReader.cpp
)

# -- libunity-scopes-qt --

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("#ifdef __clang__\n#else\n#error \"Not clang.\"\n#endif\nint main(int argc, char **argv) { return 0; }" IS_CLANG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


set(UNITY_SCOPES_QT unity-scopes-qt)
set(UNITY_SCOPES_QT_TEST ${UNITY_SCOPES_QT}-test)

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
    ${UNITY_SCOPES_QT} SHARED
    ${SCOPES_QT_SRC}
)

add_library(
    ${UNITY_SCOPES_QT_TEST} SHARED
    ${SCOPES_QT_SRC}
)

qt5_use_modules(
    ${UNITY_SCOPES_QT}

    Core
)

qt5_use_modules(
    ${UNITY_SCOPES_QT_TEST}

    Core
)

#set(LIBS unity-scopes-qt)

target_link_libraries(
    ${UNITY_SCOPES_QT}
    ${UNITY_API_LDFLAGS}
    net-cpp
)

target_link_libraries(
    ${UNITY_SCOPES_QT_TEST}
    ${UNITY_SCOPES_TEST_LIB}
    net-cpp
)

# API version
set(UNITY_SCOPES_QT_MAJOR 0)
set(UNITY_SCOPES_QT_MINOR 1)
set(UNITY_SCOPES_QT_MICRO 0)
set(UNITY_SCOPES_QT_SOVERSION ${UNITY_SCOPES_QT_MAJOR}${UNITY_SCOPES_QT_MINOR})

set_target_properties(${UNITY_SCOPES_QT} PROPERTIES
    VERSION "${UNITY_SCOPES_QT_MAJOR}.${UNITY_SCOPES_QT_MINOR}.${UNITY_SCOPES_QT_MICRO}"
    SOVERSION ${UNITY_SCOPES_QT_SOVERSION})

set_target_properties(${UNITY_SCOPES_QT_TEST} PROPERTIES
    VERSION "${UNITY_SCOPES_QT_MAJOR}.${UNITY_SCOPES_QT_MINOR}.${UNITY_SCOPES_QT_MICRO}"
    SOVERSION ${UNITY_SCOPES_QT_SOVERSION})

set(ldflags "")

# Clang sanitizers don't work if --no-undefined is given as a linker argument.
if(NOT IS_CLANG)
    set(ldflags "-Wl,--no-undefined")
endif()

set_target_properties(${UNITY_SCOPES_QT_TEST} PROPERTIES LINK_FLAGS "${ldflags}")

# export the library name so the unit test can access
set(TESTLIBS_QT ${UNITY_SCOPES_QT_TEST} CACHE STRING "Qt test library with internal symbols visible" FORCE)

install(TARGETS ${UNITY_SCOPES_QT} LIBRARY DESTINATION ${LIB_INSTALL_PREFIX})


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
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/doc-scopes-qt/index.html
                       COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile-scopes-qt
                       DEPENDS ${PROJECT_BINARY_DIR}/doc/Doxyfile-scopes-qt
                               ${CMAKE_BINARY_DIR}/include/unity/scopes/scopes-qt)
    add_custom_target(doc-scopes-qt ALL DEPENDS ${PROJECT_BINARY_DIR}/doc-scopes-qt/index.html)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc-scopes-qt/unity-scopes-qt
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc)
endif()
