add_executable(PlacementHint_test PlacementHint_test.cpp)
target_link_libraries(PlacementHint_test ${TESTLIBS})

add_test(PlacementHint PlacementHint_test)
