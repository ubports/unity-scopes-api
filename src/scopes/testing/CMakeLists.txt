set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Benchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InProcessBenchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OutOfProcessBenchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Statistics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeMetadataBuilder.cpp
)

set(UNITY_SCOPES_LIB_SRC ${UNITY_SCOPES_LIB_SRC} ${SRC} PARENT_SCOPE)
