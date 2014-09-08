include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(Registry.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Registry.ini)
configure_file(Runtime.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Runtime.ini)
configure_file(Zmq.ini.in ${CMAKE_CURRENT_BINARY_DIR}/Zmq.ini)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")

add_executable(Filters_test Filters_test.cpp)
target_link_libraries(Filters_test ${TESTLIBS})

add_test(Filters Filters_test)
