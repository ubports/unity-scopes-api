add_subdirectory(internal)
set(SLICE_FILES ${SLICE_FILES} PARENT_SCOPE)
set(CAPNPROTO_FILES ${CAPNPROTO_FILES} PARENT_SCOPE)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ActivationListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ActivationBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ActivationResponse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Annotation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CategorisedResult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Category.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CategoryRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FilterBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FilterOption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ColumnLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Department.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FilterState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Link.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ListenerBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ObjectProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OptionSelectorFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PreviewQuery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PreviewReply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PreviewWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VariantBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Query.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryCtrl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PreviewListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryMetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SearchMetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Registry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReplyBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SearchListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SearchQuery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SearchReply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scope.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeExceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeMetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Variant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp
)
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
