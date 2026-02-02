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
#include <stddef.h>     // size_t type        ISO/IEC 9899:1990 Standard
#include "qp_config.h"  // QP configuration from the application

#ifdef __cplusplus
    // no-return function specifier (C++11 Standard)
    #define Q_NORETURN  [[ noreturn ]] void

    // static assertion (C++11 Standard)
    #define Q_ASSERT_STATIC(expr_)  static_assert((expr_), "QP static assert")
#else
    // no-return function specifier (C11 Standard)
    #define Q_NORETURN   _Noreturn void

    // static assertion (C11 Standard)
    #define Q_ASSERT_STATIC(expr_)  _Static_assert((expr_), "QP static assert")
#endif

// QActive event queue type
#define QACTIVE_EQUEUE_TYPE     QEQueue
#define QACTIVE_OS_OBJ_TYPE     void *
#define QACTIVE_THREAD_TYPE     void const *

// QF interrupt disable/enable
//#define QF_INT_DISABLE()      (++QF_intLock_)
//#define QF_INT_ENABLE()       (--QF_intLock_)

// Qube critical section
#define QF_CRIT_STAT            uint_fast8_t crit_stat_;
#define QF_CRIT_ENTRY()         (crit_stat_ = QF_intLock_++)
#define QF_CRIT_EXIT()          (QF_intLock_ = crit_stat_)
#define QF_CRIT_EST()           (++QF_intLock_)

// QF_LOG2 not defined -- use the internal LOG2() implementation

// include files -------------------------------------------------------------
#include "qequeue.h"   // Qube port uses QEQueue event-queue
#include "qmpool.h"    // Qube port uses QMPool memory-pool
#include "qp.h"        // QP platform-independent public interface

void Qube_setAO(QActive *ao);
QEvt const *Qube_onGenEvt(QSignal sig, char const *params);
extern uint_fast8_t QF_intLock_;

//==========================================================================
// interface used only inside QF implementation, but not in applications
#ifdef QP_IMPL

extern QPSet QF_readySet_;
extern QPSet QF_readySet_dis;

// Qube scheduler locking (not used)
#define QF_SCHED_STAT_
#define QF_SCHED_LOCK_(dummy) ((void)0)
#define QF_SCHED_UNLOCK_()    ((void)0)

// native event queue operations
#define QACTIVE_EQUEUE_WAIT_(me_) ((void)0)
#define QACTIVE_EQUEUE_SIGNAL_(me_) \
    QPSet_insert(&QF_readySet_, (uint_fast8_t)(me_)->prio)

// QMPool operations
#define QF_EPOOL_TYPE_  QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
            (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
#define QF_EPOOL_EVENT_SIZE_(p_)  ((uint16_t)(p_).blockSize)
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
            ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
#define QF_EPOOL_PUT_(p_, e_, qsId_) (QMPool_put(&(p_), (e_), (qsId_)))
#define QF_EPOOL_USE_(ePool_)   (QMPool_getUse(ePool_))
#define QF_EPOOL_FREE_(ePool_)  ((uint16_t)(ePool_)->nFree)
#define QF_EPOOL_MIN_(ePool_)   ((uint16_t)(ePool_)->nMin)

#include "qp_pkg.h" // internal QP interface

#endif // QP_IMPL

#endif // QP_PORT_H_
