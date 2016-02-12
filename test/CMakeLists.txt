#add_subdirectory(gtest)
#add_subdirectory(copyright)
#add_subdirectory(whitespace)
#add_subdirectory(autopkg)

if(${slowtests})
#    add_subdirectory(headers)
    add_subdirectory(abi-compliance)
endif()
