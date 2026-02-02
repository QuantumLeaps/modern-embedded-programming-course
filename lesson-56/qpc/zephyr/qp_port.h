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

#include <stdint.h>        // Exact-width types. WG14/N843 C99 Standard
#include <stdbool.h>       // Boolean type.      WG14/N843 C99 Standard
#include <stddef.h>        // size_t type        ISO/IEC 9899:1990 Standard
#include "qp_config.h"     // QP configuration from the application

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

// QActive event queue and thread types
#define QACTIVE_EQUEUE_TYPE  struct k_msgq
#define QACTIVE_THREAD_TYPE  struct k_thread

// QF critical section entry/exit for Zephyr, see NOTE1
#define QF_CRIT_STAT         k_spinlock_key_t key_;
#define QF_CRIT_ENTRY()      ((key_) = k_spin_lock(&QF_spinlock))
#define QF_CRIT_EXIT()       k_spin_unlock(&QF_spinlock, key_)
#define QF_CRIT_EST()        ((void)k_spin_lock(&QF_spinlock))

// Q_PRINTK() macro to avoid conflicts with Zephyr's printk()
// when Q_SPY configuration is used
#ifndef Q_SPY
#define Q_PRINTK(fmt_, ...)  printk(fmt_, ##__VA_ARGS__)
#else
#define Q_PRINTK(dummy, ...) ((void)0)
#endif

// include files -------------------------------------------------------------
#include <zephyr/kernel.h> // Zephyr kernel API
#include "qequeue.h"       // used for event deferral
#include "qmpool.h"        // this QP port uses the native QF memory pool
#include "qp.h"            // QP platform-independent public interface

// Zephyr spinlock for QF critical section
extern struct k_spinlock QF_spinlock;

#if defined(QF_IDLE) || defined(Q_SPY)
void QF_onIdle(void);
#endif

//============================================================================
// interface used only inside QF implementation, but not in applications
#ifdef QP_IMPL

// scheduler locking, see NOTE2
#define QF_SCHED_STAT_
#define QF_SCHED_LOCK_(dummy) do { \
    if (!k_is_in_isr()) { \
        k_sched_lock();   \
    }                     \
} while (false)

#define QF_SCHED_UNLOCK_() do { \
    if (!k_is_in_isr()) { \
        k_sched_unlock(); \
    }                     \
} while (false)

// QMPool operations
#define QF_EPOOL_TYPE_  QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
            (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
#define QF_EPOOL_EVENT_SIZE_(p_)  ((uint16_t)(p_).blockSize)
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
            ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
#define QF_EPOOL_PUT_(p_, e_, qsId_) (QMPool_put(&(p_), (e_), (qsId_)))
#define QF_EPOOL_USE_(ePool_)     (QMPool_getUse(ePool_))
#define QF_EPOOL_FREE_(ePool_)    ((uint16_t)(ePool_)->nFree)
#define QF_EPOOL_MIN_(ePool_)     ((uint16_t)(ePool_)->nMin)

#endif // QP_IMPL

//============================================================================
// NOTE1:
// This QP port to Zephyr assumes that Active Objects will use only the
// preemptive Zephyr priorities [0..(CONFIG_NUM_PREEMPT_PRIORITIES - 1U)].
// In this priority numbering, the QP AO priority QF_MAX_ACTIVE (highest)
// maps to Zephyr priority 0 (highest). The QP AO priority 1 (lowest) maps
// to Zephyr priority (CONFIG_NUM_PREEMPT_PRIORITIES - 2U).
//
// NOTE2:
// Zephyr does not support selective scheduler locking up to a given
// priority ceiling. Therefore, this port uses global Zephyr scheduler lock.
//

#endif // QP_PORT_H_
