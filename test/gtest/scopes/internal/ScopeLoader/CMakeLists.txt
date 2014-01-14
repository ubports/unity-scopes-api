add_executable(ScopeLoader_test ScopeLoader_test.cpp)
target_link_libraries(ScopeLoader_test ${TESTLIBS} SharedState)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(SharedState SHARED Counters.cpp PerScopeVariables.cpp)

add_library(BadVersion MODULE BadVersion.cpp)
add_library(NoDestroy MODULE NoDestroy.cpp)
add_library(NullReturn MODULE NullReturn.cpp)
add_library(ScopeA MODULE ScopeA.cpp)
add_library(ScopeB MODULE ScopeB.cpp)
add_library(TestScope MODULE TestScope.cpp)
add_library(ThrowUnityExFromStart MODULE ThrowUnityExFromStart.cpp)
add_library(ThrowUnityExFromStop MODULE ThrowUnityExFromStop.cpp)
add_library(ThrowStdExFromStop MODULE ThrowStdExFromStop.cpp)
add_library(ThrowUnknownExFromStart MODULE ThrowUnknownExFromStart.cpp)
add_library(ThrowUnknownExFromStop MODULE ThrowUnknownExFromStop.cpp)

add_test(ScopeLoader ScopeLoader_test)
