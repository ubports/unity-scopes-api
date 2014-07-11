configure_file(testscopeA.ini.in testscopeA.ini)
configure_file(testscopeA-settings.ini.in testscopeA-settings.ini)

add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")

add_library(testscopeA MODULE testscopeA.cpp)
target_link_libraries(testscopeA ${LIBGTEST})
