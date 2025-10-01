//============================================================================
// QP/C Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QP_PORT_H_
#define QP_PORT_H_

#include <stdint.h>     // Exact-width types. WG14/N843 C99 Standard
#include <stdbool.h>    // Boolean type.      WG14/N843 C99 Standard
#include "qp_config.h"  // QP configuration from the application

// no-return function specifier (C11 Standard)
#define Q_NORETURN   _Noreturn void

// QF configuration for QK -- data members of the QActive class...

// QK event-queue used for AOs
#define QACTIVE_EQUEUE_TYPE     QEQueue

// QActive "thread" type used to store the MPU settings in the AO
#define QACTIVE_THREAD_TYPE     void const *

// interrupt disabling policy, see NOTE2 and NOTE3
#define QF_INT_DISABLE()        (QF_int_disable_())
#define QF_INT_ENABLE()         (QF_int_enable_())

// QF critical section, see NOTE1, NOTE2, and NOTE3
#define QF_CRIT_STAT
#define QF_CRIT_ENTRY()         (QF_crit_entry_())
#define QF_CRIT_EXIT()          (QF_crit_exit_())
#define QF_CRIT_EST()           (QF_crit_entry_())
#define QF_CRIT_EXIT_NOP()      __asm volatile ("isb" ::: "memory")

// Efficient log2() ----------------------------------------------------------
#if (__ARM_ARCH == 6) // ARMv6-M?
    // hand-optimized LOG2 in assembly for Cortex-M0/M0+/M1(v6-M, v6S-M)
    #define QF_LOG2(n_) QF_qlog2((uint32_t)(n_))

    // hand-optimized quick LOG2 in assembly
    uint_fast8_t QF_qlog2(uint32_t x);
#else // ARMv7-M or higher
    // ARMv7-M or higher provide the CLZ instruction for fast LOG2
    #define QF_LOG2(n_) ((uint_fast8_t)(32 - __builtin_clz((unsigned)(n_))))
#endif // ARMv7-M or higher

// Critical section policy ---------------------------------------------------
#ifdef QF_USE_BASEPRI
    // CMSIS threshold for "QF-aware" interrupts, see NOTE4
    #define QF_AWARE_ISR_CMSIS_PRI (QF_USE_BASEPRI >> (8 - __NVIC_PRIO_BITS))
#else
    // CMSIS threshold for "QF-aware" interrupts, see NOTE2 and NOTE4
    #define QF_AWARE_ISR_CMSIS_PRI 0
#endif // QF_USE_BASEPRI

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

#endif // QF_MEM_ISOLATE

// determination if the code executes in the ISR context
#define QK_ISR_CONTEXT_()     (QK_get_IPSR() != 0U)

__attribute__((always_inline))
static inline uint32_t QK_get_IPSR(void) {
    uint32_t regIPSR;
    __asm volatile ("mrs %0,ipsr" : "=r" (regIPSR));
    return regIPSR;
}

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
    // This macro should be inserted at the end of "kernel-unaware" ISRs.
    #define QK_ARM_ERRATUM_838869() __asm volatile ("dsb" ::: "memory")

#endif // ARMv6-M

// initialization of the QK kernel
#define QK_INIT()     QK_init()
void QK_init(void);
void QK_thread_ret(void);

#ifdef __ARM_FP         //--------- if VFP available...
// When the FPU is configured, clear the FPCA bit in the CONTROL register
// to prevent wasting the stack space for the FPU context.
#define QK_START()     __asm volatile ("msr CONTROL,%0" :: "r" (0) : )
#endif

// include files -------------------------------------------------------------
#include "qequeue.h"   // QK kernel uses the native QP event queue
#include "qmpool.h"    // QK kernel uses the native QP memory pool
#include "qp.h"        // QP framework
#include "qk.h"        // QK kernel

// prototypes
void QF_int_disable_(void);
void QF_int_enable_(void);
void QF_crit_entry_(void);
void QF_crit_exit_(void);

extern int32_t volatile QF_int_lock_nest_;

//============================================================================
// NOTE1:
// The critical section policy does not use the "saving and restoring"
// interrupt status policy (macro QF_CRIT_STAT is defined to nothing).
// However, this the QF critical sections might still be able to nest,
// depending on the implementation of the QF_crit_entry_()/QF_crit_exit_()
// functions. They are defined as "weak" in the qv_port.c module,
// so the application can provide a different implementation.
// Please see the definitions of the interrupt and critical-section
// functions in the qv_port.c module for details.
//
// NOTE2:
// On Cortex-M0/M0+/M1 (architecture ARMv6-M, ARMv6S-M), the interrupt
// disabling policy uses the PRIMASK register to disable interrupts globally.
// The QF_AWARE_ISR_CMSIS_PRI level is zero, meaning that all interrupts
// are "kernel-aware".
//
// NOTE3:
// If macro QF_USE_BASEPRI is defined, the interrupt disable/enable policy
// uses BASEPRI register to disable interrupts only with priority lower than
// the threshold specified by the QF_USE_BASEPRI macro. The interrupts with
// priorities above QF_USE_BASEPRI (i.e., with numerical priority values
// lower than QF_USE_BASEPRI) are NOT disabled in this method.
// These free-running interrupts have very low ("zero") latency, but they
// are NOT allowed to call any QF services, because QF is unaware of them
// ("kernel-unaware" interrupts). Consequently, only interrupts with
// numerical values of priorities equal to or higher than QF_USE_BASEPRI
// ("kernel-aware" interrupts ), can call QF services.
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

#endif // QP_PORT_H_

