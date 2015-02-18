
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ENABLE_QT_EXPERIMENTAL_")

add_definitions(
  -DFAKE_SERVER="${CMAKE_CURRENT_SOURCE_DIR}/server/server.py"
)

add_subdirectory(XmlAsyncReader)
add_subdirectory(JsonAsyncReader)
add_subdirectory(qt-bindings)
