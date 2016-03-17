add_subdirectory(IsolatedScope)
# IsolatedScopeBenchmark is too flaky and hard to fix, occasionally fails in jenkins and in VMs,
# so it's disabled.
#if (${slowtests})
#    add_subdirectory(IsolatedScopeBenchmark)
#endif()
add_subdirectory(Statistics)
