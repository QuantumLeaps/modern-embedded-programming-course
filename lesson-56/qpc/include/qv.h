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
#ifndef QV_H_
#define QV_H_

//============================================================================
//! @class QV
typedef struct {
    QPSet readySet;        //!< @private @memberof QV
    uint8_t schedCeil;     //!< @private @memberof QV
} QV;

//! @static @public @memberof QV
void QV_schedDisable(uint8_t const ceiling);

//! @static @public @memberof QV
void QV_schedEnable(void);

//! @static @public @memberof QV
void QV_onIdle(void);

//! @static @private @memberof QV
extern QV QV_priv_;

//============================================================================
// interface used only for internal implementation, but not in applications

#ifdef QP_IMPL
//! @cond INTERNAL

// scheduler locking for QV (not needed)...
#define QF_SCHED_STAT_
#define QF_SCHED_LOCK_(dummy) ((void)0)
#define QF_SCHED_UNLOCK_()    ((void)0)

// QActive event queue customization for QV...
#define QACTIVE_EQUEUE_WAIT_(me_) ((void)0)
#define QACTIVE_EQUEUE_SIGNAL_(me_) \
    QPSet_insert(&QV_priv_.readySet, (uint_fast8_t)(me_)->prio)

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

//! @endcond
#endif // QP_IMPL

#endif // QV_H_
