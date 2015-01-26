#
# Test that all header files compile stand-alone and that no public header includes an internal one.
#

set(root_inc_dir ${CMAKE_SOURCE_DIR}/include)

set(subdirs
    unity/scopes/
    unity/scopes/qt
    unity/scopes/testing
)
foreach(dir ${OTHER_INCLUDE_DIRS})
    set(other_inc_dirs "${other_inc_dirs} -I${dir}")
endforeach()

set(other_inc_dirs "${other_inc_dirs} -I${CMAKE_BINARY_DIR}/include -I${CMAKE_BINARY_DIR}")

foreach(dir ${subdirs})

    string(REPLACE "/" "-" location ${dir})

    set(public_inc_dir ${root_inc_dir}/${dir})
    set(internal_inc_dir ${public_inc_dir}/internal)

    # Test that each public header compiles stand-alone.
    add_test(stand-alone-${location}-headers
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_headers.py
            ${public_inc_dir} ${CMAKE_CXX_COMPILER} "${CMAKE_CXX_COMPILER_ARG1} -fsyntax-only -I${root_inc_dir} -I${public_inc_dir} ${other_inc_dirs} ${CMAKE_CXX_FLAGS}")

    # Test that each internal header compiles stand-alone.
    if (IS_DIRECTORY ${internal_inc_dir})
        add_test(stand-alone-${location}-internal-headers
            ${CMAKE_CURRENT_SOURCE_DIR}/compile_headers.py
                ${internal_inc_dir} ${CMAKE_CXX_COMPILER} "${CMAKE_CXX_COMPILER_ARG1} -fsyntax-only -I${root_inc_dir} -I${internal_inc_dir} ${other_inc_dirs} ${CMAKE_CXX_FLAGS}")
    endif()

    # Test that no public header includes an internal header
    add_test(clean-public-${location}-headers ${CMAKE_CURRENT_SOURCE_DIR}/check_public_headers.py ${public_inc_dir})

endforeach()

add_test(cleanincludes ${CMAKE_CURRENT_SOURCE_DIR}/includechecker.py ${CMAKE_SOURCE_DIR}/include)
add_test(noglobalincludes ${CMAKE_CURRENT_SOURCE_DIR}/check_global_usage.py
${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)
