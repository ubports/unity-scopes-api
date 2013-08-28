add_subdirectory(api)
add_subdirectory(internal)
set(SLICE_FILES ${SLICE_FILES} PARENT_SCOPE)

set(SRC
    #${CMAKE_CURRENT_SOURCE_DIR}/Exception.cpp
)
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
