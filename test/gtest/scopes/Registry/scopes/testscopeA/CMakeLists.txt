configure_file(testscopeA.ini.in testscopeA.ini)
configure_file(testscopeA-settings.ini.in testscopeA-settings.ini)

add_definitions(-DSCOPE_CONFIG_PATH="${CMAKE_CURRENT_BINARY_DIR}")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}")
add_library(testscopeA MODULE testscopeA.cpp)
set_target_properties(testscopeA
  PROPERTIES
    PREFIX "${CMAKE_LIBRARY_ARCHITECTURE}/lib"
)
target_link_libraries(testscopeA ${LIBGTEST})
add_custom_command(TARGET testscopeA POST_BUILD COMMAND nm --demangle -D $<TARGET_FILE:testscopeA>)
