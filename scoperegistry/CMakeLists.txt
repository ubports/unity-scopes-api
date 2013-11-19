set(SRC
        FindFiles.cpp
        scoperegistry.cpp
        SignalThread.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(scoperegistry ${SRC})
target_link_libraries(scoperegistry ${UNITY_SCOPES_LIB} ${OTHER_LIBS})
