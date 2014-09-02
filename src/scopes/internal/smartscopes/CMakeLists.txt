set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/SmartScope.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSQueryCtrlObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSQueryObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSRegistryObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SSScopeObject.cpp
)
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)

# -- libsmartscopes --

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

add_library(
    smartscopes STATIC

    HttpClientQt.cpp

    ${CMAKE_SOURCE_DIR}/include/unity/scopes/internal/smartscopes/HttpClientQtThread.h
    HttpClientQtThread.cpp

    SmartScopesClient.cpp
)

qt5_use_modules(
    smartscopes

    Core
    Network
)

target_link_libraries(
    smartscopes
    ${UNITY_API_LDFLAGS}
)
