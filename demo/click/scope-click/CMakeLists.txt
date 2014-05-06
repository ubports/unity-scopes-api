
if(CLICK_MODE)
    project(unity-scope-click C CXX)
    cmake_minimum_required(VERSION 2.8.10)

    include(GNUInstallDirs)
    include(FindPkgConfig)

    pkg_check_modules(UNITY_SCOPES REQUIRED lib${UNITY_SCOPES_LIB}>=0.4.0)
    add_definitions(${UNITY_SCOPES_CFLAGS} ${UNITY_SCOPES_CFLAGS_OTHER})

    add_definitions(-std=c++11)
endif(CLICK_MODE)

add_subdirectory(click)

set(symbol_map "${CMAKE_CURRENT_SOURCE_DIR}/scope-click.map")

add_library(scope-click MODULE scope-click.cpp)
# Add_dependencies should be used sparingly. In this case we need the global
# header to be generated before we start building the client binary.
add_dependencies(scope-click globalheader)
set_target_properties(scope-click PROPERTIES
                      LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(scope-click PROPERTIES LINK_DEPENDS ${symbol_map})

configure_file(scope-click.ini.in scope-click.ini)

if(CLICK_MODE)
    install(TARGETS scope-click
            DESTINATION "${CMAKE_INSTALL_PREFIX}/scope-click/")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/scope-click.ini"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/scope-click/")
endif(CLICK_MODE)
