configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_RUNTIME_PATH="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DTEST_RUNTIME_FILE="${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini")
add_definitions(-DTEST_SCOPERUNNER_PATH="${PROJECT_BINARY_DIR}/scoperunner")

add_executable(StandAloneScope_test StandAloneScope_test.cpp)
target_link_libraries(StandAloneScope_test ${TESTLIBS})
add_dependencies(StandAloneScope_test scoperunner)

add_test(StandAloneScope StandAloneScope_test)

configure_file(SimpleScope.ini.in ${CMAKE_CURRENT_BINARY_DIR}/SimpleScope.ini)
add_library(SimpleScope MODULE SimpleScope.cpp)
