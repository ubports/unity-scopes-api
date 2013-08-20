configure_file(Factory.ini.in Factory.ini)
configure_file(Runtime.ini.in Runtime.ini)
configure_file(Registry.ini.in Registry.ini)
configure_file(Ice.Registry.Config.in Ice.Registry.Config)
configure_file(Ice.Config.in Ice.Config)

add_library(scope-A MODULE scope-A.cpp)
add_library(scope-B MODULE scope-B.cpp)
add_library(scope-C MODULE scope-C.cpp)
add_library(scope-D MODULE scope-D.cpp)

add_executable(scoperunner-A scoperunner-A.cpp)
target_link_libraries(scoperunner-A ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

add_executable(scoperunner-BC scoperunner-BC.cpp)
target_link_libraries(scoperunner-BC ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

add_executable(scoperunner-D scoperunner-D.cpp)
target_link_libraries(scoperunner-D ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

add_executable(client client.cpp)
target_link_libraries(client ${UNITY_SCOPES_LIB} ${OTHER_LIBS})

configure_file(Ice.Config.in ${CMAKE_CURRENT_BINARY_DIR}/Ice.Config)
