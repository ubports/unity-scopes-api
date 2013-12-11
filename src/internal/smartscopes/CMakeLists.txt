add_subdirectory(remotescope)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

find_library(
    JSONCPP_LIBRARY
    NAMES jsoncpp
    PATHS /usr/lib /usr/local/lib
)

add_library(
    smartscopes

    HttpClientQt.cpp
    JsonCppNode.cpp

    ${CMAKE_SOURCE_DIR}/include/scopes/internal/smartscopes/HttpClientQtThread.h
    HttpClientQtThread.cpp

    SmartScopesClient.cpp
)

qt5_use_modules(
    smartscopes

    Core
    Network
    Widgets
)

target_link_libraries(
    smartscopes

    ${UNITY_API_LDFLAGS}
    ${JSONCPP_LIBRARY}
)
