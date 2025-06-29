# QPC Zephyr module
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial

zephyr_library_named(qpc)

if(CONFIG_QPC)

set(QPC_DIR ..)

zephyr_include_directories(
 ${QPC_DIR}/include
 ${QPC_DIR}/zephyr
)

target_include_directories(app PUBLIC
 ${QPC_DIR}/include
 ${QPC_DIR}/zephyr
)

zephyr_library_sources(
 ${QPC_DIR}/src/qf/qep_hsm.c
 ${QPC_DIR}/src/qf/qep_msm.c
 ${QPC_DIR}/src/qf/qf_act.c
 ${QPC_DIR}/src/qf/qf_defer.c
 ${QPC_DIR}/src/qf/qf_dyn.c
 ${QPC_DIR}/src/qf/qf_mem.c
 ${QPC_DIR}/src/qf/qf_ps.c
 ${QPC_DIR}/src/qf/qf_qact.c
 ${QPC_DIR}/src/qf/qf_qeq.c
 ${QPC_DIR}/src/qf/qf_qmact.c
 ${QPC_DIR}/src/qf/qf_time.c
 ${QPC_DIR}/zephyr/qf_port.c
)

# QSPY option...
if(QSPY)

target_compile_definitions(app PUBLIC Q_SPY)
zephyr_library_compile_definitions(Q_SPY)

zephyr_library_sources(
 ${QPC_DIR}/src/qs/qs.c
 ${QPC_DIR}/src/qs/qs_rx.c
 ${QPC_DIR}/src/qs/qs_fp.c
 ${QPC_DIR}/src/qs/qutest.c
 ${QPC_DIR}/src/qs/qs_64bit.c
 ${QPC_DIR}/src/qs/qstamp.c
)

endif() # QSPY

endif() # CONFIG_QPC
