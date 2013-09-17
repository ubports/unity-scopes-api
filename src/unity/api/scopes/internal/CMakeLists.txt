#add_subdirectory(ice_middleware)
#set(SLICE_FILES ${SLICE_FILES} PARENT_SCOPE)

add_subdirectory(zmq_middleware)
set(CAPNPROTO_FILES ${CAPNPROTO_FILES} PARENT_SCOPE)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConfigBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DynamicLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MiddlewareBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MiddlewareFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MiddlewareFactoryConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWQuery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWQueryCtrl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWReply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MWScope.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ObjectProxyImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryBaseImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryCtrlImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryCtrlObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Reaper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.cpp
)
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)

set(CONFIG_FILE_PREFIX "var/unity/api/scopes/config")

#configure_file(Registry.config.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.config)

add_executable(scope-registry RegistryMain.cpp zmq_middleware/RegistryI.cpp)
target_link_libraries(scope-registry ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

configure_file(Factory.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Factory.ini)
# configure_file(Ice.Registry.config.in ${CMAKE_CURRENT_BINARY_DIR}/Ice.Registry.config)
configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
