add_subdirectory(capnproto)
set(CAPNPROTO_FILES ${CAPNPROTO_FILES} PARENT_SCOPE)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Current.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ObjectAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryCtrlI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistryI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RequestMode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RethrowException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ServantBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SigReceiverI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VariantConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqMiddleware.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqQuery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqQueryCtrl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqReceiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqReply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqScope.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqSender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZmqSigReceiver.cpp
)
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
