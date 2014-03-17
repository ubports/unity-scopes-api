set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/SSQueryCtrlObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSQueryObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSRegistryObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSScopeObject.cpp
)
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)

# -- libsmartscopes --

pkg_check_modules(NET_CPP net-cpp REQUIRED)

include_directories(${NET_CPP_INCLUDE_DIRS})

add_library(
    smartscopes STATIC

    HttpClientNetCpp.cpp

    SmartScopesClient.cpp
)

set_target_properties(smartscopes PROPERTIES COMPILE_FLAGS "-fPIC")

target_link_libraries(
    smartscopes

    ${UNITY_API_LDFLAGS}

    ${NET_CPP_LDFLAGS}
)
