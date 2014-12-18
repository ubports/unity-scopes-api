
add_definitions(
  -DFAKE_SERVER="${CMAKE_CURRENT_SOURCE_DIR}/server/server.py"
)

add_subdirectory(XmlAsyncReader)
add_subdirectory(JsonAsyncReader)
