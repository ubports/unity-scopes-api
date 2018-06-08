set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ENABLE_QT_EXPERIMENTAL_")

set(SCOPES_QT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/HttpAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QActionMetadataImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QCannedQueryImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QCategorisedResultImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QCategoryImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QColumnLayoutImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QDepartmentImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QPreviewQueryBaseImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QPreviewReplyImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QPreviewWidgetImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QResultImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QScopeBaseAPIImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QScopeBaseImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QScopeVariant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QSearchMetadataImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QSearchQueryBaseImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QSearchReplyImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/QVariantBuilderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QActionMetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QCannedQuery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QCategorisedResult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QCategory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QColumnLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QDepartment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QPreviewQueryBaseAPI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QPreviewQueryBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QPreviewReply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QPreviewWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QResult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QScopeBaseAPI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QScopeBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QSearchMetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QSearchQueryBaseAPI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QSearchQueryBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QSearchReply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QVariantBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlAsyncReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XmlReader.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/internal/QScopeBaseAPIImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/QPreviewQueryBaseAPI.h
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/QPreviewQueryBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/QPreviewReply.h
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/QScopeBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/QSearchQueryBaseAPI.h
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/QSearchQueryBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/scopes/qt/QSearchReply.h
)

# -- libunity-scopes-qt --

include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("#ifdef __clang__\n#else\n#error \"Not clang.\"\n#endif\nint main(int argc, char **argv) { return 0; }" IS_CLANG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


set(UNITY_SCOPES_QT_TEST ${UNITY_SCOPES_QT_LIB}-test)

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
    ${UNITY_SCOPES_QT_LIB} SHARED
    ${SCOPES_QT_SRC}
)

add_library(
    ${UNITY_SCOPES_QT_TEST} SHARED
    ${SCOPES_QT_SRC}
)

target_link_libraries(${UNITY_SCOPES_QT_LIB} Qt5::Core)

target_link_libraries(${UNITY_SCOPES_QT_TEST} Qt5::Core)

#set(LIBS unity-scopes-qt)

target_link_libraries(
    ${UNITY_SCOPES_QT_LIB}
    ${UNITY_SCOPES_LIB}
    net-cpp
)

target_link_libraries(
    ${UNITY_SCOPES_QT_TEST}
    ${UNITY_SCOPES_LIB}
    net-cpp
)

set_target_properties(${UNITY_SCOPES_QT_LIB} PROPERTIES
    VERSION ${UNITY_SCOPES_QT_FULL_VERSION}
    SOVERSION ${UNITY_SCOPES_QT_SOVERSION})

set_target_properties(${UNITY_SCOPES_QT_TEST} PROPERTIES
    VERSION ${UNITY_SCOPES_QT_FULL_VERSION}
    SOVERSION ${UNITY_SCOPES_QT_SOVERSION})

set(ldflags "")

# Clang sanitizers don't work if --no-undefined is given as a linker argument.
if(NOT IS_CLANG)
    set(ldflags "-Wl,--no-undefined")
endif()

set_target_properties(${UNITY_SCOPES_QT_TEST} PROPERTIES LINK_FLAGS "${ldflags}")

# export the library name so the unit test can access
set(TESTLIBS_QT ${UNITY_SCOPES_QT_TEST} CACHE STRING "Qt test library with internal symbols visible" FORCE)
set(LIB_QT ${UNITY_SCOPES_QT_LIB} CACHE STRING "Qt library" FORCE)


# We compile with all symbols visible by default.
set(symbol_map "${CMAKE_SOURCE_DIR}/src/scopes/qt/${UNITY_SCOPES_QT_LIB}.map")
set_target_properties(${UNITY_SCOPES_QT_LIB} PROPERTIES
                      LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(${UNITY_SCOPES_QT_LIB} PROPERTIES LINK_DEPENDS ${symbol_map})

install(TARGETS ${UNITY_SCOPES_QT_LIB} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


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
                               ${CMAKE_BINARY_DIR}/include/unity/scopes/qt)
    add_custom_target(doc-scopes-qt ALL DEPENDS ${PROJECT_BINARY_DIR}/doc-scopes-qt/index.html)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc-scopes-qt/unity-scopes-qt
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc)
endif()
