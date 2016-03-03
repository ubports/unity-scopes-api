file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

install(FILES ${headers}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unity-scopes-qt-${UNITY_SCOPES_QT_SOVERSION}/unity/scopes/qt)
