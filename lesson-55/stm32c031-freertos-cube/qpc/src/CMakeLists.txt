# qpc/src
add_subdirectory(qf)
add_subdirectory(qs)
if(${PORT} IN_LIST QPC_BAREMETAL_PORTS)
    message(STATUS "adding subdir '${KERNEL}' for port '${PORT}'")
    add_subdirectory(${KERNEL})
endif()
