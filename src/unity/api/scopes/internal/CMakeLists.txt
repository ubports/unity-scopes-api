add_subdirectory(ice_middleware)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConfigBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DynamicLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MiddlewareBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MiddlewareFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MiddlewareFactoryConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWObjectProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWRegistryProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWReplyProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWScopeProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryProxyImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyProxyImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeProxyImpl.cpp
)

set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)

set(CONFIG_FILE_PREFIX "var/unity/api/scopes/config")

configure_file(Registry.config.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.config)

add_executable(scope-registry RegistryMain.cpp ice_middleware/RegistryI.cpp)
target_link_libraries(scope-registry ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

configure_file(Factory.ini.in Factory.ini)
configure_file(Registry.ini.in Registry.ini)
configure_file(Scopes.ini.in Scopes.ini)
