set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferedResultForwarderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferedSearchReplyImpl.cpp
)

set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
