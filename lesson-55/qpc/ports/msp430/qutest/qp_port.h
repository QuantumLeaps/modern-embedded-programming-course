//============================================================================
// QP/C Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: LicenseRef-QL-commercial
//
// This software is licensed under the terms of the Quantum Leaps commercial
// licenses. Please contact Quantum Leaps for more information about the
// available licensing options.
//
// RESTRICTIONS
// You may NOT :
// (a) redistribute, encumber, sell, rent, lease, sublicense, or otherwise
//     transfer rights in this software,
// (b) remove or alter any trademark, logo, copyright or other proprietary
//     notices, legends, symbols or labels present in this software,
// (c) plagiarize this software to sidestep the licensing obligations.
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

#if defined(__TI_COMPILER_VERSION__) || defined(__GNUC__)
    // no-return function specifier (C11 Standard)
    #define Q_NORETURN   _Noreturn void
#elif defined(__IAR_SYSTEMS_ICC__)
    #define Q_NORETURN   __noreturn void
#endif

// QF configuration for QK -- data members of the QActive class...

// QActive event queue type
#define QACTIVE_EQUEUE_TYPE     QEQueue
// QACTIVE_OS_OBJ_TYPE not used in this port
// QACTIVE_THREAD_TYPE not used in this port

// QF critical section
#define QF_CRIT_STAT         QCritStatus critStat_;
#define QF_CRIT_ENTRY()      (critStat_ = QF_critEntry())
#define QF_CRIT_EXIT()       (QF_critExit(critStat_))
#define QF_CRIT_EST()        ((void)QF_critEntry())

// QF_LOG2 not defined -- use the internal LOG2() implementation

// include files -------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h>
#elif defined(__GNUC__)
    #include <msp430.h>
    #include "in430.h"
#endif

#include "qequeue.h"   // QUTest port uses QEQueue event-queue
#include "qmpool.h"    // QUTest port uses QMPool memory-pool
#include "qp.h"        // QP platform-independent public interface

//============================================================================
// interface used only inside QF implementation, but not in applications

#ifdef QP_IMPL

    // QUTest scheduler locking
    #define QF_SCHED_STAT_ QSchedStatus lockStat_;
    #define QF_SCHED_LOCK_(ceil_) (lockStat_ = QF_schedLock((ceil_)))
    #define QF_SCHED_UNLOCK_()    (QF_schedUnlock(lockStat_))

    // native QEQueue operations
    #define QACTIVE_EQUEUE_WAIT_(me_) ((void)0)
    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        QPSet_insert(&QS_tstPriv_.readySet, (uint_fast8_t)(me_)->prio)

    // native QMPool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((uint_fast16_t)(p_).blockSize)
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
        ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
    #define QF_EPOOL_PUT_(p_, e_, qsId_) \
        (QMPool_put(&(p_), (e_), (qsId_)))

#endif // QP_IMPL

#endif // QP_PORT_H_

