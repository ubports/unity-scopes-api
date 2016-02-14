add_subdirectory(gtest)
add_subdirectory(copyright)
add_subdirectory(whitespace)
add_subdirectory(autopkg)

if(${slowtests})
    add_subdirectory(headers)
    # TODO: Disabled 9 Feb 2016 because abi-compliance-checker cannot handle the latest Cap'n Proto
    #       headers that are in xenial-proposed. Fix this URGENTLY!!!
    # add_subdirectory(abi-compliance)  # Keep this test last because it's really slow.
endif()
