set(
  SRC

  ${CMAKE_CURRENT_SOURCE_DIR}/UnityScopesApi_tp.cpp
)

set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)

add_library(
  lttngtracer SHARED

  UnityScopesApi_tp.c
)

target_link_libraries(
  lttngtracer

  lttng-ust
  urcu-bp
  dl
)
