configure_file(abi.xml.in ${CMAKE_CURRENT_BINARY_DIR}/abi.xml)
configure_file(check-abi-compliance.sh.in ${CMAKE_CURRENT_BINARY_DIR}/check-abi-compliance.sh @ONLY)

add_test(check-abi-compliance /bin/sh check-abi-compliance.sh)
