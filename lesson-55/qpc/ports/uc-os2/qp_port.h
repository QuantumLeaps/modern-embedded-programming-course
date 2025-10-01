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

// uC-OS2 event queue and thread types
#define QACTIVE_EQUEUE_TYPE     OS_EVENT *
#define QACTIVE_THREAD_TYPE     uint32_t

// include files -------------------------------------------------------------
#include "ucos_ii.h"   // uC-OS2 API, port and compile-time configuration

#include "qequeue.h"   // QP event queue (for deferring events)
#include "qmpool.h"    // QP memory pool (for event pools)
#include "qp.h"        // QP platform-independent public interface

// uC-OS2 critical section, NOTE1
#if (OS_CRITICAL_METHOD == 1U)
    #define QF_CRIT_STAT
    #define QF_CRIT_ENTRY()  OS_ENTER_CRITICAL()
    #define QF_CRIT_EXIT()   OS_EXIT_CRITICAL()
    #define QF_CRIT_EST()    OS_ENTER_CRITICAL()
#elif (OS_CRITICAL_METHOD == 3U)
    #define QF_CRIT_STAT     OS_CPU_SR cpu_sr;
    #define QF_CRIT_ENTRY()  OS_ENTER_CRITICAL()
    #define QF_CRIT_EXIT()   OS_EXIT_CRITICAL()
    #define QF_CRIT_EST()    do { \
        OS_CPU_SR cpu_sr;         \
        OS_ENTER_CRITICAL();      \
        (void)cpu_sr;             \
    } while (false)
#else
    #error Unsupported uC-OS2 critical section type
#endif // OS_CRITICAL_METHOD


enum UCOS2_TaskAttrs {
    TASK_NAME_ATTR
};

//============================================================================
// interface used only inside QF implementation, but not in applications
#ifdef QP_IMPL

    // uC-OS2-specific scheduler locking, see NOTE2
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) do { \
        if (OSIntNesting == 0U) {      \
            OSSchedLock();             \
        }                              \
    } while (false)

    #define QF_SCHED_UNLOCK_() do { \
        if (OSIntNesting == 0U) {   \
            OSSchedUnlock();        \
        }                           \
    } while (false)

    // native QF event pool customization
    #define QF_EPOOL_TYPE_        QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((uint_fast16_t)(p_).blockSize)
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
        ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
    #define QF_EPOOL_PUT_(p_, e_, qsId_) \
        (QMPool_put(&(p_), (e_), (qsId_)))

#endif // QP_IMPL

//============================================================================
// NOTE1:
// This QP port to uC-OS2 re-uses the exact same critical section mechanism
// as uC-OS2. The goal is to make this port independent on the CPU or the
// toolchain by employing only the official uC-OS2 API. That way, all CPU
// and toolchain dependencies are handled internally by uC-OS2.
//
// NOTE2:
// uC-OS2 provides only global scheduler locking for all thread priorities
// by means of OSSchedLock() and OSSchedUnlock(). Therefore, locking the
// scheduler only up to the specified lock priority is not supported.
//

#endif // QP_PORT_H_

