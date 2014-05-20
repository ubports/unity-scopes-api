set(SRC
        DirWatch.cpp
        FindFiles.cpp
        scoperegistry.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(scoperegistry ${SRC})
target_link_libraries(scoperegistry ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

install(TARGETS scoperegistry RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${UNITY_SCOPES_LIB})
