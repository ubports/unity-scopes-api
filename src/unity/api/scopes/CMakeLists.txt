add_subdirectory(internal)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ObjectProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeExceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp
)

set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
