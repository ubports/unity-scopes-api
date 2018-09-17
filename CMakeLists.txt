cmake_minimum_required(VERSION 2.8.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default install location. Must be set here, before setting the project.
if (NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "" FORCE)
endif()

set(PROJECT unity-scopes-api)
project(${PROJECT} C CXX)

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

set(ACCEPTED_BUILD_TYPES "" none release debug relwithdebinfo coverage)
list(FIND ACCEPTED_BUILD_TYPES "${cmake_build_type_lower}" IS_BUILD_TYPE_ACCEPTED)
if(${IS_BUILD_TYPE_ACCEPTED} EQUAL -1)
  message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}\nValid types are: ${ACCEPTED_BUILD_TYPES}")
endif()

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
find_package(Boost COMPONENTS system filesystem regex serialization thread REQUIRED)
pkg_check_modules(UNITY_API libunity-api>=0.1.3 REQUIRED)
pkg_check_modules(PROCESS_CPP process-cpp>=1.0.1 REQUIRED)
pkg_check_modules(APPARMOR REQUIRED libapparmor REQUIRED)
pkg_check_modules(LTTNG_UST lttng-ust REQUIRED)
pkg_check_modules(LIBURCU_BP liburcu-bp REQUIRED)
pkg_check_modules(LIBACCOUNTS libaccounts-glib REQUIRED)
pkg_check_modules(LIBSIGNON libsignon-glib REQUIRED)
pkg_check_modules(ZMQLIB libzmq REQUIRED)
pkg_check_modules(NET_CPP net-cpp>=1.2.0 REQUIRED)
pkg_check_modules(JSON_GLIB json-glib-1.0 REQUIRED)

find_library(ZMQPPLIB zmqpp)
if(NOT ZMQPPLIB)
  message(FATAL_ERROR "libzmqpp-dev not found.")
endif()

find_library(CAPNPLIB capnp)
if(NOT CAPNPLIB)
  message(FATAL_ERROR "libcapnp-dev not found.")
endif()

find_library(KJLIB kj)
if(NOT KJLIB)
  message(FATAL_ERROR "capnproto not found.")
endif()

find_library(DLLIB dl)
if(NOT DLLIB)
  message(FATAL_ERROR "dl lib not found.")
endif()

find_program(LTTNG_EXECUTABLE lttng)
if (NOT LTTNG_EXECUTABLE)
    message(SEND_ERROR "Cannot find LTTng executable: ensure that lttng-tools is installed")
endif()

#
# Code style fixer. We put the code through astyle first because it makes some fixes that
# clang-format won't do (such as "char *p" -> "char* p"). But astyle messes up other things
# (particularly lambdas and assembly-style comments), which clang-format does right.
# So, we run clang-format after running astyle, which undoes the damage done by astyle
# without reverting desirable astyle fixes.
#

find_program(ASTYLE_COMMAND NAMES astyle)
if (NOT ASTYLE_COMMAND)
    message(WARNING "Cannot find astyle: formatcode target will not be available")
else()
    # astyle 2.03 creates DOS line endings, so we need to fix its output
    find_program(DOS2UNIX_COMMAND NAMES dos2unix)
    if (NOT DOS2UNIX_COMMAND)
        message(WARNING "Cannot find dos2unix: formatcode target will not be available")
    else()
        # Some time after clang-format-3.6, someone finally decided to install the versioned
        # /usr/bin/clang-format binary name as /usr/bin/clang-format (without the version suffix).
        find_program(CLANG_FORMAT_COMMAND NAMES clang-format clang-format-3.6 clang-format-3.5)
        if (NOT CLANG_FORMAT_COMMAND)
            message(WARNING "Cannot find clang-format >= clang-format-3.5: formatcode target will not be available")
        endif()
    endif()
endif()

if (ASTYLE_COMMAND AND DOS2UNIX_COMMAND AND CLANG_FORMAT_COMMAND)
set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${fmt_h})
    add_custom_target(formatcode
                      ${PROJECT_SOURCE_DIR}/tools/format-files.sh ${PROJECT_SOURCE_DIR} ${ASTYLE_COMMAND} ${CLANG_FORMAT_COMMAND})
endif()

find_program(CAPNPC_BIN capnpc)
if(NOT CAPNPC_BIN)
  message(FATAL_ERROR "Capnp compiler not found.")
endif()
set(CAPNPC_FLAGS "" CACHE STRING "Extra flags for capnpc.")

add_definitions(
  -DDEB_HOST_MULTIARCH="${CMAKE_LIBRARY_ARCHITECTURE}"
)

set(OTHER_INCLUDE_DIRS ${OTHER_INCLUDE_DIRS} ${UNITY_API_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                       ${JSON_GLIB_INCLUDE_DIRS} ${PROCESS_CPP_INCLUDE_DIRS} ${APPARMOR_INCLUDE_DIRS}
                       ${LIBACCOUNTS_INCLUDE_DIRS} ${LIBSIGNON_INCLUDE_DIRS} ${NET_CPP_INCLUDE_DIRS})
set(OTHER_LIBS ${OTHER_LIBS} ${UNITY_API_LDFLAGS} ${APPARMOR_LDFLAGS} ${LIBACCOUNTS_LIBRARIES}
               ${LIBSIGNON_LIBRARIES} ${Boost_LIBRARIES} ${JSON_GLIB_LDFLAGS} ${PROCESS_CPP_LDFLAGS}
               ${ZMQPPLIB} ${ZMQLIB_LDFLAGS} ${CAPNPLIB} ${KJLIB} ${DLLIB} ${NET_CPP_LDFLAGS} pthread)

# Standard install paths
include(GNUInstallDirs)

include_directories(
    include/
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/src
    ${OTHER_INCLUDE_DIRS}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-variadic-macros -Wextra -fPIC")

# By default, all symbols are visible in the library. We strip out things we don't
# want at link time, by running a version script (see unity-scopes.map and the
# setting of LINK_FLAGS below).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default")

set(C_AND_CXX_WARNINGS "-pedantic -Wall -Wextra")

# Some additional warnings not included by the general flags set above.
set(EXTRA_C_WARNINGS "-Wcast-align -Wcast-qual -Wformat -Wredundant-decls -Wswitch-default")
set(EXTRA_CXX_WARNINGS "-Wnon-virtual-dtor -Wctor-dtor-privacy -Wold-style-cast")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_AND_CXX_WARNINGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_WARNINGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${C_AND_CXX_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_C_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_WARNINGS}")

# -fno-permissive causes warnings with clang, so we only enable it for gcc
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-permissive")
endif()

if ("${cmake_build_type_lower}" STREQUAL "release" OR "${cmake_build_type_lower}" STREQUAL "relwithdebinfo")
    option(Werror "Treat warnings as errors" ON)
else()
    option(Werror "Treat warnings as errors" OFF)
endif()

# Some tests are slow, so make it possible not to run them during day-to-day development.
# The default for this *must* stay ON, otherwise the tests won't run in Jenkins.
option(slowtests "Run slow tests" ON)

if (${Werror})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    if ("${cmake_build_type_lower}" STREQUAL "release" OR "${cmake_build_type_lower}" STREQUAL "relwithdebinfo")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=deprecated-declarations")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")
    endif()
endif()

set(SANITIZER "" CACHE STRING "Build with -fsanitize=<value> (legal values: thread, address)")

if ("${SANITIZER}" STREQUAL "")
    # Do nothing
elseif (${SANITIZER} STREQUAL "thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer")
elseif (${SANITIZER} STREQUAL "address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
else()
    message(FATAL_ERROR "Invalid SANITIZER setting: ${SANITIZER}")
endif()

# Scopes library names. Also used as a prefix in various places, and as a subdirectory
# name for installed files. Changing this will also require corresponding changes
# to the debian files.
set(UNITY_SCOPES_LIB unity-scopes)
set(UNITY_SCOPES_QT_LIB unity-scopes-qt)

set(DEBIAN_DIR ${CMAKE_SOURCE_DIR}/debian)

# Set versions for libunity-scopes.
file(READ ${DEBIAN_DIR}/VERSION version)
string(REGEX MATCHALL "[0-9]+" version_list ${version})
list(GET version_list 0 UNITY_SCOPES_MAJOR)
list(GET version_list 1 UNITY_SCOPES_MINOR)
list(GET version_list 2 UNITY_SCOPES_MICRO)

set(UNITY_SCOPES_MAJOR_MINOR ${UNITY_SCOPES_MAJOR}.${UNITY_SCOPES_MINOR})
set(UNITY_SCOPES_FULL_VERSION ${UNITY_SCOPES_MAJOR_MINOR}.${UNITY_SCOPES_MICRO})

execute_process(COMMAND lsb_release -c -s OUTPUT_VARIABLE SERIES)
if(NOT SERIES STREQUAL "")
    string(STRIP ${SERIES} SERIES)
endif()
if (SERIES STREQUAL "vivid")
    # For backward compatibility. On Vivid, the soname started counting at 3: libunity-scopes.so.3.
    math(EXPR UNITY_SCOPES_SOVERSION "${UNITY_SCOPES_MINOR} + 3")
else()
    # For Wily and later, the soname is libunity-scopes.so.<maj>.<min>, e.g. libunity-scopes.so.1.0.
    set(UNITY_SCOPES_SOVERSION ${UNITY_SCOPES_MAJOR_MINOR})
endif()

# Set versions for libunity-scopes-qt.
file(READ ${DEBIAN_DIR}/QT-VERSION version)
string(REGEX MATCHALL "[0-9]+" version_list ${version})
list(GET version_list 0 UNITY_SCOPES_QT_MAJOR)
list(GET version_list 1 UNITY_SCOPES_QT_MINOR)
list(GET version_list 2 UNITY_SCOPES_QT_MICRO)

set(UNITY_SCOPES_QT_MAJOR_MINOR ${UNITY_SCOPES_QT_MAJOR}.${UNITY_SCOPES_QT_MINOR})
set(UNITY_SCOPES_QT_FULL_VERSION ${UNITY_SCOPES_QT_MAJOR_MINOR}.${UNITY_SCOPES_QT_MICRO})
set(UNITY_SCOPES_QT_SOVERSION ${UNITY_SCOPES_QT_MAJOR_MINOR})

if(NOT ${UNITY_SCOPES_MAJOR} EQUAL 1)
    message(FATAL_ERROR "Change in major version number. You need to adjust bileto_pre_release_hook")
endif()

if(NOT ${UNITY_SCOPES_QT_MAJOR} EQUAL 0)
    message(FATAL_ERROR "Change in Qt major version number. You need to adjust bileto_pre_release_hook")
endif()

# Library for testing, with all symbols visible
set(UNITY_SCOPES_TEST_LIB ${UNITY_SCOPES_LIB}-test)

# All the libraries we need to link a normal executable that uses Unity scopes
set(LIBS ${UNITY_SCOPES_LIB})

# Test version of ${UNITY_SCOPES_LIB}, with internal symbols visible
set(TESTLIBS ${UNITY_SCOPES_TEST_LIB})

# Library install prefix
set(LIB_INSTALL_PREFIX lib/${CMAKE_LIBRARY_ARCHITECTURE})
set(LIBDIR ${CMAKE_INSTALL_LIBDIR})

set(LIBSUBDIR ${LIBDIR}/${UNITY_SCOPES_LIB}})
set(HDR_INSTALL_DIR include/${UNITY_SCOPES_LIB}-${UNITY_SCOPES_SOVERSION})

#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage (or ninja if you use that backend)
#  * Find html report in subdir coveragereport
#  * Find xml report suitable for jenkins in coverage.xml
#####################################################################
if(cmake_build_type_lower MATCHES coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -g")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage -g")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -g")
endif()
include(EnableCoverageReport)

# Tests
include(CTest)
enable_testing()

# Add subdirectories.
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(scoperegistry)
add_subdirectory(scoperunner)
add_subdirectory(smartscopesproxy)
add_subdirectory(data)
add_subdirectory(test)
add_subdirectory(demo)
add_subdirectory(tools)

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

# Pseudo-library of object files. We need a version of the library for normal clients
# that does not expose symbols in the unity::scopes::internal namespace, and another
# version for whitebox tests, so we can write unit tests for classes in the internal namespaces.
# Here, we create an object library that then is used to link the normal and the test libraries.

set(UNITY_SCOPES_LIB_OBJ ${UNITY_SCOPES_LIB}-obj)
add_library(${UNITY_SCOPES_LIB_OBJ} OBJECT ${UNITY_SCOPES_LIB_SRC} ${CAPNPROTO_SRC})
set_target_properties(${UNITY_SCOPES_LIB_OBJ} PROPERTIES COMPILE_FLAGS "-fPIC")

# Use the object files to make the normal library.
add_library(${UNITY_SCOPES_LIB} SHARED $<TARGET_OBJECTS:${UNITY_SCOPES_LIB_OBJ}>)
set_target_properties(${UNITY_SCOPES_LIB} PROPERTIES
    VERSION ${UNITY_SCOPES_FULL_VERSION}
    SOVERSION ${UNITY_SCOPES_SOVERSION}
)

# Use the object files to make the test library.
add_library(${UNITY_SCOPES_TEST_LIB} SHARED $<TARGET_OBJECTS:${UNITY_SCOPES_LIB_OBJ}>)
set_target_properties(${UNITY_SCOPES_TEST_LIB} PROPERTIES
    VERSION ${UNITY_SCOPES_FULL_VERSION}
    SOVERSION ${UNITY_SCOPES_SOVERSION}
)

set(ldflags "")

# Clang sanitizers don't work if --no-undefined is given as a linker argument.
if(NOT IS_CLANG)
    set(ldflags "-Wl,--no-undefined")
endif()

set_target_properties(${UNITY_SCOPES_TEST_LIB} PROPERTIES LINK_FLAGS "${ldflags}")

# We compile with all symbols visible by default. For the shipping library, we strip
# out all symbols that (recursively) are in the unity::scopes::internal namespace,
# except for a few exceptions that are needed by the scoperegistry, scoperunner,
# smartscopesproxy, and any other binaries that reach into the internal namespace.
set(symbol_map "${CMAKE_SOURCE_DIR}/${UNITY_SCOPES_LIB}.map")
set_target_properties(${UNITY_SCOPES_LIB} PROPERTIES
                      LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(${UNITY_SCOPES_LIB} PROPERTIES LINK_DEPENDS ${symbol_map})

target_link_libraries(${UNITY_SCOPES_LIB} ${OTHER_LIBS})
target_link_libraries(${UNITY_SCOPES_TEST_LIB} ${OTHER_LIBS})

# Only the normal library gets installed.
install(TARGETS ${UNITY_SCOPES_LIB} LIBRARY DESTINATION ${LIB_INSTALL_PREFIX})

# Enable coverage testing

if (cmake_build_type_lower MATCHES coverage)
  ENABLE_COVERAGE_REPORT(TARGETS ${UNITY_SCOPES_LIB}
                         FILTER /usr/include
                                ${CMAKE_SOURCE_DIR}/test/*
                                ${CMAKE_SOURCE_DIR}/debian/tests/*
                                ${CMAKE_SOURCE_DIR}/demo/*
                                ${CMAKE_BINARY_DIR}/*)
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
      message(STATUS "Creating devel documentation")
      configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile-devel.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY IMMEDIATE)
    else()
      configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY IMMEDIATE)
    endif()
    configure_file(${PROJECT_SOURCE_DIR}/doc/index.html ${PROJECT_BINARY_DIR}/doc/index.html COPYONLY)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/doc/${UNITY_SCOPES_LIB}/index.html
                       COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile
                       DEPENDS ${PROJECT_BINARY_DIR}/doc/Doxyfile
                               ${PROJECT_SOURCE_DIR}/doc/tutorial.dox
                               ${UNITY_SCOPES_LIB_SRC}
                               ${UNITY_SCOPES_LIB_HDRS})
    add_custom_target(doc ALL
                       DEPENDS ${PROJECT_BINARY_DIR}/doc/${UNITY_SCOPES_LIB}/index.html)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/${UNITY_SCOPES_LIB}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc)
endif()
