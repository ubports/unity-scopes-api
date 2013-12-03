cmake_minimum_required(VERSION 2.8)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(
    smartscopes

    HttpClientInterface.h
    HttpClientQt.cpp
    HttpClientQt.h
    JsonCppParser.cpp
    JsonCppParser.h
    JsonParserInterface.h

    SmartScopesClient.cpp
    SmartScopesClient.h
)

qt5_use_modules(
    smartscopes

    Core
    Network
    Widgets
)

target_link_libraries(
    smartscopes

    jsoncpp
)
