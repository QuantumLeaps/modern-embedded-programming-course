//============================================================================
// QP/C Real-Time Embedded Framework (RTEF)
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2023-08-17
//! @version Last updated for: @ref qpc_7_3_0
//!
//! @file
//! @brief QV/C port to ARM Cortex-M, IAR-ARM

#define QP_IMPL 1U
#include "qp_port.h"

#define SCB_SYSPRI   ((uint32_t volatile *)0xE000ED18U)
#define NVIC_IP      ((uint32_t volatile *)0xE000E400U)
#define SCB_CPACR   *((uint32_t volatile *)0xE000ED88U)
#define FPU_FPCCR   *((uint32_t volatile *)0xE000EF34U)

//..........................................................................
// Initialize the exception priorities and IRQ priorities to safe values.
//
// Description:
// On ARMv7-M or higher, this QK port disables interrupts by means of the
// BASEPRI register. However, this method cannot disable interrupt
// priority zero, which is the default for all interrupts out of reset.
// The following code changes the SysTick priority and all IRQ priorities
// to the safe value QF_BASEPRI, which the QF critical section can disable.
// This avoids breaching of the QF critical sections in case the
// application programmer forgets to explicitly set priorities of all
// "kernel aware" interrupts.
//
// The interrupt priorities established in QV_init() can be later
// changed by the application-level code.
void QV_init(void) {

#if (__ARM_ARCH != 6)   //--------- if ARMv7-M and higher...

    // SCB_SYSPRI[2]:  SysTick
    SCB_SYSPRI[2] = (SCB_SYSPRI[2] | (QF_BASEPRI << 24U));

    // set all 240 possible IRQ priories to QF_BASEPRI...
    for (uint_fast8_t n = 0U; n < (240U/sizeof(uint32_t)); ++n) {
        NVIC_IP[n] = (QF_BASEPRI << 24U) | (QF_BASEPRI << 16U)
                     | (QF_BASEPRI << 8U) | QF_BASEPRI;
    }

#endif                  //--------- ARMv7-M or higher

#ifdef __ARM_FP         //--------- if VFP available...
    // make sure that the FPU is enabled by seting CP10 & CP11 Full Access
    SCB_CPACR = (SCB_CPACR | ((3UL << 20U) | (3UL << 22U)));

    // FPU automatic state preservation (ASPEN) lazy stacking (LSPEN)
    FPU_FPCCR = (FPU_FPCCR | (1U << 30U) | (1U << 31U));
#endif                  //--------- VFP available
}

//============================================================================
#if (__ARM_ARCH == 6) // if ARMv6-M...

// hand-optimized quick LOG2 in assembly (no CLZ instruction in ARMv6-M)
uint_fast8_t QF_qlog2(uint32_t x) {
    static uint8_t const log2LUT[16] = {
        0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
        4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
    };
    uint_fast8_t n;

__asm volatile (
   "  MOVS    %[n],#0           \n"
#if (QF_MAX_ACTIVE > 16U)
    "  LSRS    r2,r0,#16        \n"
    "  BEQ     QF_qlog2_1       \n"
    "  MOVS    %[n],#16         \n"
    "  MOVS    r0,r2            \n"
    "QF_qlog2_1:                \n"
#endif
#if (QF_MAX_ACTIVE > 8U)
    "  LSRS    r2,r0,#8         \n"
    "  BEQ     QF_qlog2_2       \n"
    "  ADDS    %[n],%[n],#8     \n"
    "  MOVS    r0,r2            \n"
    "QF_qlog2_2:                \n"
#endif
    "  LSRS    r2,r0,#4         \n"
    "  BEQ.N   QF_qlog2_3       \n"
    "  ADDS    %[n],%[n],#4     \n"
    "  MOVS    r0,r2            \n"
    "QF_qlog2_3:" : [n]"=r"(n)
);
    return n + log2LUT[x];
}

#endif // ARMv6-M

