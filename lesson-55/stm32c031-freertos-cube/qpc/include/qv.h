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
typedef struct QV {
    //! @cond INTERNAL
    uint8_t dummy;
    //! @endcond
} QV;

//============================================================================
//! @class QV_Attr
typedef struct {
    QPSet readySet;        //!< @private @memberof QV_Attr
    uint8_t schedCeil;     //!< @private @memberof QV_Attr
} QV_Attr;

//! @static @private @memberof QV
extern QV_Attr QV_priv_;

//! @static @public @memberof QV
void QV_schedDisable(uint_fast8_t const ceiling);

//! @static @public @memberof QV
void QV_schedEnable(void);

//! @static @public @memberof QV
void QV_onIdle(void);

//============================================================================
// interface used only for internal implementation, but not in applications
#ifdef QP_IMPL

// scheduler locking for QV (not needed)...
#define QF_SCHED_STAT_
#define QF_SCHED_LOCK_(dummy) ((void)0)
#define QF_SCHED_UNLOCK_()    ((void)0)

// QActive event queue customization for QV...
#define QACTIVE_EQUEUE_WAIT_(me_) ((void)0)
#define QACTIVE_EQUEUE_SIGNAL_(me_) \
    QPSet_insert(&QV_priv_.readySet, (uint_fast8_t)(me_)->prio)

// QF event pool customization for QV...
#define QF_EPOOL_TYPE_ QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
#define QF_EPOOL_EVENT_SIZE_(p_) ((uint_fast16_t)(p_).blockSize)
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
    ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
#define QF_EPOOL_PUT_(p_, e_, qsId_) (QMPool_put(&(p_), (e_), (qsId_)))

#endif // QP_IMPL

#endif // QV_H_
