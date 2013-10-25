set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/LttngLogger.cpp
)
set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)

add_library(
  lttnglogger

  LttngLogger_tp.c
)

target_link_libraries(
  lttnglogger

  lttng-ust
  urcu-bp
)
