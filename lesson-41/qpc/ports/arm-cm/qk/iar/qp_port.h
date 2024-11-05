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
//! @date Last updated on: 2023-09-07
//! @version Last updated for: @ref qpc_7_3_0
//!
//! @file
//! @brief QP/C port to ARM Cortex-M, preemptive QK kernel, IAR-ARM

#ifndef QP_PORT_H_
#define QP_PORT_H_

#include <stdint.h>  // Exact-width types. WG14/N843 C99 Standard
#include <stdbool.h> // Boolean type.      WG14/N843 C99 Standard
#include <intrinsics.h> // IAR intrinsic functions

#ifdef QP_CONFIG
#include "qp_config.h" // external QP configuration
#endif

// no-return function specifier (C11 Standard)
#define Q_NORETURN   _Noreturn void

// QF configuration for QK -- data members of the QActive class...

// QK event-queue used for AOs
#define QACTIVE_EQUEUE_TYPE     QEQueue

// QF "thread" type used to store the MPU settings in the AO
#define QACTIVE_THREAD_TYPE     void const *

// QF interrupt disable/enable and log2()...
#if (__ARM_ARCH == 6) // ARMv6-M?

    // Cortex-M0/M0+/M1(v6-M, v6S-M) interrupt disabling policy, see NOTE2
    #define QF_INT_DISABLE()    __disable_interrupt()
    #define QF_INT_ENABLE()     __enable_interrupt()

    // QF critical section entry/exit (save and restore interrupt status)
    #define QF_CRIT_STAT        unsigned long primask_;
    #define QF_CRIT_ENTRY() do {      \
        (primask_) = __get_PRIMASK(); \
        QF_INT_DISABLE(); \
    } while (false)
    #define QF_CRIT_EXIT()      __set_PRIMASK((primask_))

    // CMSIS threshold for "QF-aware" interrupts, see NOTE2 and NOTE4
    #define QF_AWARE_ISR_CMSIS_PRI 0

    // hand-optimized LOG2 in assembly for Cortex-M0/M0+/M1(v6-M, v6S-M)
    #define QF_LOG2(n_) QF_qlog2((uint32_t)(n_))

#else // ARMv7-M or higher

    // ARMv7-M or higher alternative interrupt disabling with PRIMASK
    #define QF_PRIMASK_DISABLE() __disable_interrupt()
    #define QF_PRIMASK_ENABLE()  __enable_interrupt()

    // ARMv7-M or higher interrupt disabling policy, see NOTE3 and NOTE4
    #define QF_INT_DISABLE() do {  \
        QF_PRIMASK_DISABLE();      \
        __set_BASEPRI(QF_BASEPRI); \
        QF_PRIMASK_ENABLE();       \
    } while (false)
    #define QF_INT_ENABLE()      __set_BASEPRI(0U)

    // QF critical section entry/exit (save and restore interrupt status)
    #define QF_CRIT_STAT         unsigned long basepri_;
    #define QF_CRIT_ENTRY() do {    \
        basepri_ = __get_BASEPRI(); \
        QF_INT_DISABLE();           \
    } while (false)
    #define QF_CRIT_EXIT()       __set_BASEPRI((basepri_))

    // BASEPRI threshold for "QF-aware" interrupts, see NOTE3
    #define QF_BASEPRI          0x3F

    // CMSIS threshold for "QF-aware" interrupts, see NOTE5
    #define QF_AWARE_ISR_CMSIS_PRI (QF_BASEPRI >> (8 - __NVIC_PRIO_BITS))

    // ARMv7-M or higher provide the CLZ instruction for fast LOG2
    #define QF_LOG2(n_) ((uint_fast8_t)(32U - __CLZ((unsigned long)(n_))))

#endif

#define QF_CRIT_EXIT_NOP()      __ISB()

#if (__ARM_ARCH == 6) // ARMv6-M?
    // hand-optimized quick LOG2 in assembly
    uint_fast8_t QF_qlog2(uint32_t x);
#endif // ARMv7-M or higher

// Memory isolation ----------------------------------------------------------
#ifdef QF_MEM_ISOLATE

    // Memory isolation requires the context-switch
    #define QF_ON_CONTEXT_SW   1U

    // Memory System setting
    #define QF_MEM_SYS() QF_onMemSys()

    // Memory Application setting
    #define QF_MEM_APP() QF_onMemApp()

    // callback functions for memory settings (provided by applications)
    void QF_onMemSys(void);
    void QF_onMemApp(void);

#endif // def QF_MEM_ISOLATE

// determination if the code executes in the ISR context
#define QK_ISR_CONTEXT_()     (__get_IPSR() != 0U)

// QK ISR entry and exit
#define QK_ISR_ENTRY() ((void)0)

#ifdef QF_MEM_ISOLATE
    #define QK_ISR_EXIT()  do {                                   \
        QF_INT_DISABLE();                                         \
        QF_MEM_SYS();                                             \
        if (QK_sched_() != 0U) {                                  \
            *Q_UINT2PTR_CAST(uint32_t, 0xE000ED04U) = (1U << 28U);\
        }                                                         \
        QF_MEM_APP();                                             \
        QF_INT_ENABLE();                                          \
        QK_ARM_ERRATUM_838869();                                  \
    } while (false)
#else
    #define QK_ISR_EXIT()  do {                                   \
        QF_INT_DISABLE();                                         \
        if (QK_sched_() != 0U) {                                  \
            *Q_UINT2PTR_CAST(uint32_t, 0xE000ED04U) = (1U << 28U);\
        }                                                         \
        QF_INT_ENABLE();                                          \
        QK_ARM_ERRATUM_838869();                                  \
    } while (false)
#endif

#if (__ARM_ARCH == 6) // ARMv6-M?
    #define QK_ARM_ERRATUM_838869() ((void)0)
#else // ARMv7-M or higher
    // The following macro implements the recommended workaround for the
    // ARM Erratum 838869. Specifically, for Cortex-M3/M4/M7 the DSB
    // (memory barrier) instruction needs to be added before exiting an ISR.
    #define QK_ARM_ERRATUM_838869() __DSB()

#endif // ARMv6-M

// initialization of the QK kernel
#define QK_INIT() QK_init()
void QK_init(void);
void QK_thread_ret(void);

#ifdef __ARM_FP         //--------- if VFP available...
// When the FPU is configured, clear the FPCA bit in the CONTROL register
// to prevent wasting the stack space for the FPU context.
#define QK_START()     __set_CONTROL(0U)
#endif

// include files -------------------------------------------------------------
#include "qequeue.h"   // QK kernel uses the native QP event queue
#include "qmpool.h"    // QK kernel uses the native QP memory pool
#include "qp.h"        // QP framework
#include "qk.h"        // QK kernel

//============================================================================
// NOTE2:
// On Cortex-M0/M0+/M1 (architecture v6-M, v6S-M), the interrupt disabling
// policy uses the PRIMASK register to disable interrupts globally. The
// QF_AWARE_ISR_CMSIS_PRI level is zero, meaning that all interrupts are
// "QF-aware".
//
// NOTE3:
// On ARMv7-M or higher, the interrupt disable/enable policy uses the BASEPRI
// register (which is not implemented in Cortex-M0/M0+/M1) to disable
// interrupts only with priority lower than the threshold specified by the
// QF_BASEPRI macro. The interrupts with priorities above QF_BASEPRI (i.e.,
// with numerical priority values lower than QF_BASEPRI) are NOT disabled in
// this method. These free-running interrupts have very low ("zero") latency,
// but they are not allowed to call any QF services, because QF is unaware
// of them ("QF-unaware" interrupts). Consequently, only interrupts with
// numerical values of priorities equal to or higher than QF_BASEPRI
// ("QF-aware" interrupts ), can call QF services.
//
// NOTE4:
// The QF_AWARE_ISR_CMSIS_PRI macro is useful as an offset for enumerating
// the "QF-aware" interrupt priorities in the applications, whereas the
// numerical values of the "QF-aware" interrupts must be greater or equal to
// QF_AWARE_ISR_CMSIS_PRI. The values based on QF_AWARE_ISR_CMSIS_PRI can be
// passed directly to the CMSIS function NVIC_SetPriority(), which shifts
// them by (8 - __NVIC_PRIO_BITS) into the correct bit position, while
// __NVIC_PRIO_BITS is the CMSIS macro defining the number of implemented
// priority bits in the NVIC. Please note that the macro QF_AWARE_ISR_CMSIS_PRI
// is intended only for applications and is not used inside the QF port, which
// remains generic and not dependent on the number of implemented priority bits
// implemented in the NVIC.
//
// NOTE5:
// The selective disabling of "QF-aware" interrupts with the BASEPRI register
// has a problem on ARM Cortex-M7 core r0p1 (see ARM-EPM-064408, errata
// 837070). The workaround recommended by ARM is to surround MSR BASEPRI with
// the CPSID i/CPSIE i pair, which is implemented in the QF_INT_DISABLE()
// macro. This workaround works also for Cortex-M3/M4 cores.

#endif // QP_PORT_H_

