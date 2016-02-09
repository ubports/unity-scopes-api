configure_file(check-abi-compliance.sh.in ${CMAKE_CURRENT_BINARY_DIR}/check-abi-compliance.sh @ONLY)
configure_file(gen-abi-dump.sh.in ${CMAKE_CURRENT_BINARY_DIR}/gen-abi-dump.sh @ONLY)

set(abi_dump_cmd /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/gen-abi-dump.sh)
set(abi_check_cmd /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/check-abi-compliance.sh)
set(abi_dump_file ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}_${UNITY_SCOPES_FULL_VERSION}.abi.xml)
set(abi_report ${CMAKE_CURRENT_BINARY_DIR}/lib${UNITY_SCOPES_LIB}_${UNITY_SCOPES_FULL_VERSION}_abi-report.txt)
message(${abi_report})

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

install(FILES ${abi_dump_file} DESTINATION ${LIB_INSTALL_PREFIX})
