add_subdirectory(remotescope)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

find_path(JSONCPP_INCLUDE_DIR json/json.h
          PATH_SUFFIXES include/jsoncpp jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIR})

find_library(JSONCPP_LIBRARY REQUIRED jsoncpp)

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

    ${JSONCPP_LIBRARY}
)
