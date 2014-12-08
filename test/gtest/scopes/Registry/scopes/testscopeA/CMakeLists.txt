configure_file(testscopeA.ini.in testscopeA.ini)
configure_file(testscopeA-settings.ini.in testscopeA-settings.ini)

add_definitions(-DSCOPE_CONFIG_PATH="${CMAKE_CURRENT_BINARY_DIR}")

add_library(testscopeA MODULE testscopeA.cpp)
set_target_properties(testscopeA
  PROPERTIES
    PREFIX "${DEB_HOST_MULTIARCH}/lib"
)
target_link_libraries(testscopeA ${LIBGTEST})
