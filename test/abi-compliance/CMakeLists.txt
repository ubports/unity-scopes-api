configure_file(abi.xml.in ${CMAKE_CURRENT_BINARY_DIR}/abi.xml)
configure_file(check-abi-compliance.sh.in ${CMAKE_CURRENT_BINARY_DIR}/check-abi-compliance.sh @ONLY)
configure_file(gen-abi-dump.sh.in ${CMAKE_CURRENT_BINARY_DIR}/gen-abi-dump.sh @ONLY)

set(abi_dump_cmd /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/gen-abi-dump.sh)
set(abi_check_cmd /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/check-abi-compliance.sh)
set(abi_dump_file ${CMAKE_CURRENT_BINARY_DIR}/abi_dumps/lib${UNITY_SCOPES_LIB}/lib${UNITY_SCOPES_LIB}_${UNITY_SCOPES_FULL_VERSION}.abi.tar.gz)
set(abi_report ${CMAKE_CURRENT_BINARY_DIR}/compat_reports/lib${UNITY_SCOPES_LIB}/lib${UNITY_SCOPES_LIB}/${UNITY_SCOPES_MAJOR_MINOR}.0_to_${UNITY_SCOPES_FULL_VERSION}/compat_report.html)

add_custom_command(
    OUTPUT ${abi_dump_file}
    COMMAND ${abi_dump_cmd}
    DEPENDS ${UNITY_SCOPES_LIB})

add_custom_target(gen-abi-dump DEPENDS ${abi_dump_file})

add_custom_command(
    OUTPUT ${abi_report}
    COMMAND ${abi_check_cmd}
    DEPENDS gen-abi-dump)

add_custom_target(check-abi DEPENDS ${abi_report})

add_test(check-abi-compliance ${CMAKE_MAKE_PROGRAM} check-abi)
