add_subdirectory(unity)

# Pseudo-library of object files. We need a dynamic version of the library for normal clients,
# and a static version for the whitebox tests, so we can write unit tests for classes in the internal namespaces
# (because, for the .so, non-public APIs are compiled with -fvisibility=hidden).
# Everything is compiled with -fPIC, so the same object files are suitable for either library.
# Here, we create an object library that then is used to link the static and dynamic
# libraries, without having to compile each source file twice with different compile flags.

set(UNITY_SCOPES_LIB_OBJ ${UNITY_SCOPES_LIB}-obj)
add_library(${UNITY_SCOPES_LIB_OBJ} OBJECT ${UNITY_SCOPES_LIB_SRC})
set_target_properties(${UNITY_SCOPES_LIB_OBJ} PROPERTIES COMPILE_FLAGS "-fPIC")

# Use the object files to make the shared library.
set(UNITY_SCOPES_SOVERSION 0)
add_library(${UNITY_SCOPES_LIB} SHARED $<TARGET_OBJECTS:${UNITY_SCOPES_LIB_OBJ}>)
set_target_properties(${UNITY_SCOPES_LIB} PROPERTIES
    VERSION "${UNITY_SCOPES_MAJOR}.${UNITY_SCOPES_MINOR}"
    SOVERSION ${UNITY_SCOPES_SOVERSION}
)

# Use the object files to make the static library.
add_library(${UNITY_SCOPES_STATIC_LIB} STATIC $<TARGET_OBJECTS:${UNITY_SCOPES_LIB_OBJ}>)
set_target_properties(${UNITY_SCOPES_STATIC_LIB} PROPERTIES OUTPUT_NAME ${UNITY_SCOPES_LIB})

# Only the dynamic library gets installed.
install(TARGETS ${UNITY_SCOPES_LIB} LIBRARY DESTINATION ${LIB_INSTALL_PREFIX})

# Set up package config.
configure_file(lib${UNITY_SCOPES_LIB}.pc.in lib${UNITY_SCOPES_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}.pc DESTINATION ${LIB_INSTALL_PREFIX}/pkgconfig)

set (SRC
)

# Parent needs to know what all the source files are, for generating doc and the like.
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
