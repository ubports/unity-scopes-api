add_subdirectory(lttng)
add_subdirectory(zmq_middleware)

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

configure_file(DfltConfig.h.in DfltConfig.h)
set(headers ${headers} ${CMAKE_CURRENT_BINARY_DIR}/DfltConfig.h)

set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${headers} PARENT_SCOPE)
