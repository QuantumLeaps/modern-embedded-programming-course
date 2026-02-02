# QPC Zephyr example
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(dpp)

target_sources(app PRIVATE
 src/main.c
 src/bsp.c
 src/philo.c
 src/table.c
)

# QPC framework directory
# NOTE:
# Adjust this path for a different relative path
# between your project and the QPC folder
#
set(QPC_DIR ../../..)

# option to turn QSPY software tracing ON or OFF (file prj.conf)
# NOTE:
# change of the option might require re-building the project
#
option(QSPY "QSPY software tracing" OFF)

# QPC framework includes and sources...
zephyr_include_directories(
 ${QPC_DIR}/include
 ${QPC_DIR}/zephyr
 ./src
)
target_sources(app PRIVATE
 ${QPC_DIR}/src/qf/qep_hsm.c
 ${QPC_DIR}/src/qf/qep_msm.c
 ${QPC_DIR}/src/qf/qf_act.c
 ${QPC_DIR}/src/qf/qf_qact.c
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

# application-level "idle thread"...
if(QIDLE)
add_compile_definitions(QF_IDLE)
endif()

# QPCPP configuration with QSPY...
if(QSPY)
add_compile_definitions(Q_SPY)
add_compile_definitions(QF_IDLE)

target_sources(app PRIVATE
 ${QPC_DIR}/src/qs/qs.c
 ${QPC_DIR}/src/qs/qs_rx.c
 ${QPC_DIR}/src/qs/qs_fp.c
 ${QPC_DIR}/src/qs/qs_64bit.c
 ${QPC_DIR}/src/qs/qstamp.c
)

endif()

