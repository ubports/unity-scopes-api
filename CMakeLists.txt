cmake_minimum_required(VERSION 2.8.10)

# Default install location. Must be set here, before setting the project.
if (NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "" FORCE)
endif()

project(unity-scopes-api C CXX)

if(${PROJECT_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
   message(FATAL_ERROR "In-tree build attempt detected, aborting. Set your build dir outside your source dir, delete CMakeCache.txt from source root and try again.")
endif()

include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("#ifdef __clang__\n#else\n#error \"Not clang.\"\n#endif\nint main(int argc, char **argv) { return 0; }" IS_CLANG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

if(IS_CLANG)
  message(STATUS "Compiling with Clang, disabling lttng.")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_lower) # Build types should always be lowercase but sometimes they are not.

# Static C++ checks

find_program(CPPCHECK_COMMAND NAMES cppcheck)
if (CPPCHECK_COMMAND)
    set(CPPCHECK_COMMAND_OPTIONS --check-config --inline-suppr --enable=all -q --error-exitcode=2)
    set(CPPCHECK_COMMAND_OPTIONS ${CPPCHECK_COMMAND_OPTIONS} --template "'{file}({line}): {severity} ({id}): {message}'")
    add_custom_target(cppcheck COMMAND ${CPPCHECK_COMMAND} ${CPPCHECK_COMMAND_OPTIONS}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/test
        ${CMAKE_BINARY_DIR}/test
    )
else()
    message(WARNING "Cannot find cppcheck: cppcheck target will not be available")
endif()

#
# Definitions for testing with valgrind.
#

configure_file(CTestCustom.cmake.in CTestCustom.cmake) # Tests in CTestCustom.cmake are skipped for valgrind

find_program(MEMORYCHECK_COMMAND NAMES valgrind)
if (MEMORYCHECK_COMMAND)
    set(MEMORYCHECK_COMMAND_OPTIONS
        "--suppressions=${CMAKE_SOURCE_DIR}/valgrind-suppress --leak-check=full --num-callers=40 --error-exitcode=3"
    )
    add_custom_target(valgrind DEPENDS NightlyMemCheck)
else()
    message(WARNING "Cannot find valgrind: valgrind target will not be available")
endif()

include(FindPkgConfig)
find_package(Boost COMPONENTS regex REQUIRED)
pkg_check_modules(UNITY_API libunity-api>=0.1.1 REQUIRED)
pkg_check_modules(LTTNG_UST lttng-ust REQUIRED)
pkg_check_modules(LIBURCU_BP liburcu-bp REQUIRED)
find_program(LTTNG_EXECUTABLE lttng)
if (NOT LTTNG_EXECUTABLE)
    message(SEND_ERROR "Cannot find LTTng executable: ensure that lttng-tools is installed")
endif()

find_program(CAPNPC_BIN capnpc)
if(NOT CAPNPC_BIN)
  message(FATAL_ERROR "Capnp compiler not found.")
endif()
set(CAPNPC_FLAGS "" CACHE STRING "Extra flags for capnpc.")

set(OTHER_INCLUDE_DIRS ${OTHER_INCLUDE_DIRS} ${UNITY_API_INCLUDE_DIRS})
set(OTHER_LIBS ${UNITY_API_LDFLAGS} ${OTHER_LIBS})

# Standard install paths
include(GNUInstallDirs)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/src
    ${OTHER_INCLUDE_DIRS}
)

# When building the library, we set the default symbol visibility
# to "hidden", so we don't export things by default.
# Exported functions and classes are prefixed by a UNITY_API macro,
# which explicitly exports a symbol if UNITY_DLL_EXPORTS is defined.
add_definitions(-DUNITY_DLL_EXPORTS)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-variadic-macros -Wextra -fPIC")

# TODO: static linking of the tests doesn't work because the tests won't pull in virtual methods in a
# base class from a static library without an unresolved dependency. But, without static linking, we can't
# do whitebox tests because of the visibility=hidden compile flags. For now, we link dynamically and
# with visbility=default.
# Longer term, we can either build two libraries each time, one with hidden and one with default visibility,
# and link the tests against the default visibility one, but the demos against the hidden one. Problem is that
# this will compile everything twice for every build.
# Or we can build a single library with visibility=default in debug and coverage mode, and build the library
# with visibliity=hidden only in release mode. But this means that we can't run the tests against the
# library build with release mode.

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default")   # HACK

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Wextra") # Can't use -std=c++11 with C compiler

# -fno-permissive causes warnings with clang, so we only enable it for gcc
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-permissive")
endif()

if ("${cmake_build_type_lower}" STREQUAL "release" OR "${cmake_build_type_lower}" STREQUAL "relwithdebinfo")
    option(Werror "Treat warnings as errors" ON)
else()
    option(Werror "Treat warnings as errors" OFF)
endif()

if (Werror)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    if ("${cmake_build_type_lower}" STREQUAL "release" OR "${cmake_build_type_lower}" STREQUAL "relwithdebinfo")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=deprecated-declarations")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")
    endif()
endif()

option(THREAD_SANITIZER "Build with -fsanitize=thread (for Clang only)" OFF)

if (IS_CLANG AND THREAD_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

option(IPC_MONITORING "Enable IPC traffic monitoring")

if(IPC_MONITORING)
    add_definitions(-DENABLE_IPC_MONITOR)
endif()

set(SCOPES_DEFAULT_DATADIR ${CMAKE_INSTALL_DATADIR}/unity)
set(SCOPES_DEFAULT_CONFIGDIR ${SCOPES_DEFAULT_DATADIR}/mw-config)

# API version
set(UNITY_SCOPES_MAJOR 0)
set(UNITY_SCOPES_MINOR 1)
set(UNITY_SCOPES_MICRO 6)

# Scopes library
set(UNITY_SCOPES_LIB unity-scopes)

# Static version for testing
set(UNITY_SCOPES_STATIC_LIB unity-scopes-static)

find_library(ZMQLIB zmq)
if(NOT ZMQLIB)
  message(FATAL_ERROR "Zmq lib not found.")
endif()

find_library(ZMQPPLIB zmqpp)
if(NOT ZMQPPLIB)
  message(FATAL_ERROR "Zmqpp lib not found.")
endif()

find_library(CAPNPLIB capnp)
if(NOT CAPNPLIB)
  message(FATAL_ERROR "Cap'n Proto lib not found.")
endif()

find_library(KJLIB kj)
if(NOT KJLIB)
  message(FATAL_ERROR "Kj lib not found.")
endif()

find_library(DLLIB dl)
if(NOT DLLIB)
  message(FATAL_ERROR "Dl lib not found.")
endif()


# Other libraries we depend on
set(OTHER_LIBS
    ${OTHER_LIBS}
    ${ZMQPPLIB}
    ${ZMQLIB}
    ${CAPNPLIB}
    ${KJLIB}
    ${DLLIB}
    pthread
    smartscopes
)

# All the libraries we need to link a normal executable that uses Unity scopes
set(LIBS ${UNITY_SCOPES_LIB})

# All the libraries we need to link a gtest executable. (We link the tests against a static version
# so we can do whitebox testing on internal classes.

# TODO: HACK: static linking of the tests doesn't work because the tests won't pull in virtual methods in a
# base class from a static library without an unresolved dependency. But, without static linking, we can't
# do whitebox tests because of the visibility=hidden compile flags. For now, we link dynamically and
# with visbility=default.

# set(TESTLIBS ${UNITY_SCOPES_STATIC_LIB})
set(TESTLIBS ${UNITY_SCOPES_LIB}) # HACK

# Library install prefix
set(LIB_INSTALL_PREFIX lib/${CMAKE_LIBRARY_ARCHITECTURE})

set(LIBDIR ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_PREFIX} CACHE PATH "Destination install dir for the library")

# Tests
include(CTest)
enable_testing()

# Add subdirectories.
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(scoperegistry)
add_subdirectory(scoperunner)
add_subdirectory(data)
add_subdirectory(test)
add_subdirectory(demo)

# Custom rules to compile .capnp files
foreach(file ${CAPNPROTO_FILES})

    string(REPLACE "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" build_dir ${file})
    get_filename_component(build_dir ${build_dir} PATH)

    get_filename_component(proto_file ${file} NAME)
    string(REPLACE ".capnp" ".capnp.c++" src_file ${proto_file})
    string(REPLACE ".capnp" ".capnp.h" hdr_file ${proto_file})

    set(src_file ${build_dir}/${src_file})
    set(hdr_file ${build_dir}/${hdr_file})

    set(CAPNPROTO_HDRS ${CAPNPROTO_HDRS} ${hdr_file})
    set(CAPNPROTO_SRC ${CAPNPROTO_SRC} ${src_file})
endforeach()

# cmake cannot analyze include file dependencies of .capnproto files, so the easiest option is to recompile
# all files if any one of them changes
add_custom_command(OUTPUT ${CAPNPROTO_SRC} ${CAPNPROTO_HDRS}
                   DEPENDS ${CAPNPROTO_FILES}
                   COMMAND ${CAPNPC_BIN} ${CAPNPC_FLAGS} -oc++:${CMAKE_BINARY_DIR} --src-prefix=${CMAKE_SOURCE_DIR} ${CAPNPROTO_FILES})

# Set up coverage testing
include(EnableCoverageReport)
#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage (or ninja if you use that backend)
#  * Find html report in subdir coveragereport
#  * Find xml report suitable for jenkins in coverage.xml
#####################################################################
if(cmake_build_type_lower MATCHES coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

    # We add -g when building with coverage so valgrind reports line numbers.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g" )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" )
endif()

# Pseudo-library of object files. We need a dynamic version of the library for normal clients,
# and a static version for the whitebox tests, so we can write unit tests for classes in the internal namespaces
# (because, for the .so, non-public APIs are compiled with -fvisibility=hidden).
# Everything is compiled with -fPIC, so the same object files are suitable for either library.
# Here, we create an object library that then is used to link the static and dynamic
# libraries, without having to compile each source file twice with different compile flags.

set(UNITY_SCOPES_LIB_OBJ ${UNITY_SCOPES_LIB}-obj)
add_library(${UNITY_SCOPES_LIB_OBJ} OBJECT ${UNITY_SCOPES_LIB_SRC} ${SLICE_SRC} ${CAPNPROTO_SRC})
set_target_properties(${UNITY_SCOPES_LIB_OBJ} PROPERTIES COMPILE_FLAGS "-fPIC")

# Use the object files to make the shared library.
set(UNITY_SCOPES_SOVERSION 0)
add_library(${UNITY_SCOPES_LIB} SHARED $<TARGET_OBJECTS:${UNITY_SCOPES_LIB_OBJ}>)
set_target_properties(${UNITY_SCOPES_LIB} PROPERTIES
    VERSION "${UNITY_SCOPES_MAJOR}.${UNITY_SCOPES_MINOR}"
    SOVERSION ${UNITY_SCOPES_SOVERSION}
)

# Clang sanitizers don't work if --no-undefined is given as a linker argument.
if(NOT IS_CLANG)
  set_target_properties(${UNITY_SCOPES_LIB} PROPERTIES
  LINK_FLAGS "-Wl,--no-undefined")
endif()

target_link_libraries(${UNITY_SCOPES_LIB} ${OTHER_LIBS})

# Use the object files to make the static library.
add_library(${UNITY_SCOPES_STATIC_LIB} STATIC $<TARGET_OBJECTS:${UNITY_SCOPES_LIB_OBJ}>)
set_target_properties(${UNITY_SCOPES_STATIC_LIB} PROPERTIES OUTPUT_NAME ${UNITY_SCOPES_LIB})

# Only the dynamic library gets installed.
install(TARGETS ${UNITY_SCOPES_LIB} LIBRARY DESTINATION ${LIB_INSTALL_PREFIX})

set_target_properties(${UNITY_SCOPES_LIB} PROPERTIES
    VERSION "${UNITY_SCOPES_MAJOR}.${UNITY_SCOPES_MINOR}"
    SOVERSION ${UNITY_SCOPES_SOVERSION}
)

# Enable coverage testing

if (cmake_build_type_lower MATCHES coverage)
  ENABLE_COVERAGE_REPORT(TARGETS ${UNITY_SCOPES_LIB} smartscopes FILTER /usr/include ${CMAKE_SOURCE_DIR}/test/* ${CMAKE_BINARY_DIR}/*)
endif()

#
# Documentation
#
# Pass -DDEVEL_DOCS=ON to cmake for more detailed documentation

option(DEVEL_DOCS "Enable detailed Doxygen documentation")

find_package(Doxygen)
find_program(DOT_EXECUTABLE dot /usr/bin)
if (NOT DOXYGEN_FOUND OR NOT DOT_EXECUTABLE)
    message(WARNING "Cannot generate documentation: doxygen and/or graphviz not found")
else()
    if (DEVEL_DOCS)
      message(STATUS "Creaing devel documentation")
      configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile-devel.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY IMMEDIATE)
    else()
      configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY IMMEDIATE)
    endif()
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/doc/lib${UNITY_SCOPES_LIB}/index.html
                       COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile
                       DEPENDS ${PROJECT_BINARY_DIR}/doc/Doxyfile
                               ${UNITY_SCOPES_LIB_SRC}
                               ${UNITY_SCOPES_LIB_HDRS})
    add_custom_target(doc ALL
                       DEPENDS ${PROJECT_BINARY_DIR}/doc/lib${UNITY_SCOPES_LIB}/index.html)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/lib${UNITY_SCOPES_LIB}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc)
endif()
