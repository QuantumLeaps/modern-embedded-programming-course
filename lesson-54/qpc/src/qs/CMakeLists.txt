# ./src/qs
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qs.c)
    target_sources(qpc PRIVATE
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_64bit.c>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_fp.c>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_rx.c>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qstamp.c>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs.c>"
        "$<$<AND:$<CONFIG:Spy>,$<BOOL:${QPC_CFG_UNIT_TEST}>>:${CMAKE_CURRENT_SOURCE_DIR}/qutest.c>"
    )
else()
    target_sources(qpc PRIVATE
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qstamp.c>"
    )
endif()
