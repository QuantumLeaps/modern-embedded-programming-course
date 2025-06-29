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
#ifndef QK_H_
#define QK_H_

//============================================================================
//! @class QK
typedef struct QK {
    //! @cond INTERNAL
    uint8_t dummy;
    //! @endcond
} QK;

typedef uint_fast8_t QSchedStatus;

//============================================================================
//! @class QK_Attr
typedef struct {
    QPSet readySet;        //!< @memberof QK_Attr
    uint8_t actPrio;       //!< @memberof QK_Attr
    uint8_t nextPrio;      //!< @memberof QK_Attr
    uint8_t actThre;       //!< @memberof QK_Attr
    uint8_t lockCeil;      //!< @memberof QK_Attr
    uint8_t intNest;       //!< @memberof QK_Attr
} QK_Attr;

//! @static @private @memberof QK
extern QK_Attr QK_priv_;

//! @static @private @memberof QK
uint_fast8_t QK_sched_(void);

//! @static @private @memberof QK
uint_fast8_t QK_sched_act_(
    QActive const * const act,
    uint_fast8_t const pthre_in);

//! @static @private @memberof QK
void QK_activate_(void);

//! @static @public @memberof QK
QSchedStatus QK_schedLock(uint_fast8_t const ceiling);

//! @static @public @memberof QK
void QK_schedUnlock(QSchedStatus const prevCeil);

//! @static @public @memberof QK
void QK_onIdle(void);

//============================================================================
// interface used only for internal implementation, but not in applications
#ifdef QP_IMPL

// scheduler locking for QK...
#define QF_SCHED_STAT_ QSchedStatus lockStat_;
#define QF_SCHED_LOCK_(ceil_) do { \
    if (QK_ISR_CONTEXT_()) { \
        lockStat_ = 0xFFU; \
    } else { \
        lockStat_ = QK_schedLock((ceil_)); \
    } \
} while (false)

#define QF_SCHED_UNLOCK_() do { \
    if (lockStat_ != 0xFFU) { \
        QK_schedUnlock(lockStat_); \
    } \
} while (false)

// QActive event queue customization for QK...
#define QACTIVE_EQUEUE_WAIT_(me_) ((void)0)
#define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    QPSet_insert(&QK_priv_.readySet, (uint_fast8_t)(me_)->prio); \
    if (!QK_ISR_CONTEXT_()) { \
        if (QK_sched_() != 0U) { \
            QK_activate_(); \
        } \
    } \
} while (false)

// QF event pool customization for QK...
#define QF_EPOOL_TYPE_ QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
#define QF_EPOOL_EVENT_SIZE_(p_) ((uint_fast16_t)(p_).blockSize)
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
    ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
#define QF_EPOOL_PUT_(p_, e_, qsId_) \
    (QMPool_put(&(p_), (e_), (qsId_)))

#endif // QP_IMPL

#endif // QK_H_
