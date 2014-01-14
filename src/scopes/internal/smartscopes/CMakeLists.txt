add_subdirectory(remotescope)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
pkg_check_modules(JSONCPP jsoncpp REQUIRED)

include_directories(${JSONCPP_INCLUDE_DIRS})

if(NOT JSONCPP_FOUND)
  message(FATAL_ERROR "Dependency jsoncpp not found.")
endif()

add_library(
    smartscopes SHARED

    HttpClientQt.cpp
    JsonCppNode.cpp

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
    ${JSONCPP_LDFLAGS}
)
