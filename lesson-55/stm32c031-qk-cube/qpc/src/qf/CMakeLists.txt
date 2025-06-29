# ./src/qf
target_sources(qpc PRIVATE
    qep_hsm.c
    qep_msm.c
    qf_act.c
    # qf_actq.c - see below
    qf_defer.c
    qf_dyn.c
    # qf_mem.c - see below
    qf_ps.c
    qf_qact.c
    qf_qeq.c
    qf_qmact.c
    qf_time.c
)
if(NOT (${QPC_CFG_PORT} IN_LIST QPC_RTOS_PORTS))
    target_sources(qpc PRIVATE
        qf_actq.c
    )
endif()
if(NOT (${QPC_CFG_PORT} STREQUAL uc-os2))
    target_sources(qpc PRIVATE
        qf_mem.c
    )
endif()