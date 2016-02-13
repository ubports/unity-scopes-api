configure_file(check-abi-compliance.sh.in ${CMAKE_CURRENT_BINARY_DIR}/check-abi-compliance.sh @ONLY)
configure_file(gen-abi-dump.sh.in ${CMAKE_CURRENT_BINARY_DIR}/gen-abi-dump.sh @ONLY)

set(abi_dump_cmd /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/gen-abi-dump.sh)
set(abi_check_cmd /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/check-abi-compliance.sh)
set(abi_dump_file ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}_${UNITY_SCOPES_FULL_VERSION}.abi.xml)
set(abi_baseline_dump ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}_${UNITY_SCOPES_MAJOR}.0.0.abi.xml)
set(abi_report ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}_${UNITY_SCOPES_FULL_VERSION}_abi-report.txt)

add_custom_command(
    OUTPUT ${abi_dump_file}
    COMMAND ${abi_dump_cmd}
    DEPENDS ${UNITY_SCOPES_LIB})

add_custom_target(gen-abi-dump DEPENDS ${abi_dump_file})

add_custom_command(
    OUTPUT run_always
    COMMAND ${abi_check_cmd}
    DEPENDS ${abi_dump_file} ${suppressions})
set_source_files_properties(run_always
    PROPERTIES SYMBOLIC true)
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES ${abi_report}
    ADDITIONAL_MAKE_CLEAN_FILES ${abi_baseline_dump})

add_custom_target(check-abi DEPENDS run_always)

add_test(check-abi-compliance ${CMAKE_MAKE_PROGRAM} check-abi)
