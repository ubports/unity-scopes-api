configure_file(complete_config.ini.in complete_config.ini)
configure_file(mandatory_config.ini.in mandatory_config.ini)
configure_file(ttl_small.ini.in ttl_small.ini)
configure_file(ttl_medium.ini.in ttl_medium.ini)
configure_file(bad_timeout.ini.in bad_timeout.ini)
configure_file(bad_ttl.ini.in bad_ttl.ini)
configure_file(bad_version.ini.in bad_version.ini)
configure_file(bad_child_ids.ini.in bad_child_ids.ini)
configure_file(empty_child_ids.ini.in empty_child_ids.ini)
configure_file(single_child_id.ini.in single_child_id.ini)
configure_file(bad_keywords.ini.in bad_keywords.ini)
configure_file(empty_keywords.ini.in empty_keywords.ini)
configure_file(single_keyword.ini.in single_keyword.ini)
configure_file(empty_display_name.ini.in empty_display_name.ini)
configure_file(empty_description.ini.in empty_description.ini)
configure_file(empty_author.ini.in empty_author.ini)

add_definitions(-DCOMPLETE_CONFIG="${CMAKE_CURRENT_BINARY_DIR}/complete_config.ini")
add_definitions(-DMANDATORY_CONFIG="${CMAKE_CURRENT_BINARY_DIR}/mandatory_config.ini")
add_definitions(-DTTL_SMALL="${CMAKE_CURRENT_BINARY_DIR}/ttl_small.ini")
add_definitions(-DTTL_MEDIUM="${CMAKE_CURRENT_BINARY_DIR}/ttl_medium.ini")
add_definitions(-DBAD_TIMEOUT="${CMAKE_CURRENT_BINARY_DIR}/bad_timeout.ini")
add_definitions(-DBAD_CHILD_IDS="${CMAKE_CURRENT_BINARY_DIR}/bad_child_ids.ini")
add_definitions(-DEMPTY_CHILD_IDS="${CMAKE_CURRENT_BINARY_DIR}/empty_child_ids.ini")
add_definitions(-DSINGLE_CHILD_ID="${CMAKE_CURRENT_BINARY_DIR}/single_child_id.ini")
add_definitions(-DBAD_TTL="${CMAKE_CURRENT_BINARY_DIR}/bad_ttl.ini")
add_definitions(-DBAD_VERSION="${CMAKE_CURRENT_BINARY_DIR}/bad_version.ini")
add_definitions(-DBAD_KEYWORDS="${CMAKE_CURRENT_BINARY_DIR}/bad_keywords.ini")
add_definitions(-DEMPTY_KEYWORDS="${CMAKE_CURRENT_BINARY_DIR}/empty_keywords.ini")
add_definitions(-DSINGLE_KEYWORD="${CMAKE_CURRENT_BINARY_DIR}/single_keyword.ini")
add_definitions(-DEMPTY_DISPLAY_NAME="${CMAKE_CURRENT_BINARY_DIR}/empty_display_name.ini")
add_definitions(-DEMPTY_DESCRIPTION="${CMAKE_CURRENT_BINARY_DIR}/empty_description.ini")
add_definitions(-DEMPTY_AUTHOR="${CMAKE_CURRENT_BINARY_DIR}/empty_author.ini")

add_executable(ScopeConfig_test ScopeConfig_test.cpp)
target_link_libraries(ScopeConfig_test ${TESTLIBS})

add_test(ScopeConfig ScopeConfig_test)
