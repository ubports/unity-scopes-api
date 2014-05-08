set(SRC scoperunner.cpp)

add_executable(scoperunner ${SRC})
target_link_libraries(scoperunner ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

install(TARGETS scoperunner RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${UNITY_SCOPES_LIB})
