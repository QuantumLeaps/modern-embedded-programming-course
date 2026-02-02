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
#define QP_IMPL           // this is QP implementation
#include "qp_port.h"      // QP port
#include "qp_pkg.h"       // QP package-scope interface
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // QS port
    #include "qs_pkg.h"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

#if (QF_MAX_EPOOL > 0U)   // mutable events configured?

Q_DEFINE_THIS_MODULE("qf_dyn")

//............................................................................
//! @static @public @memberof QF
void QF_poolInit(
    void * const poolSto,
    uint_fast32_t const poolSize,
    uint_fast16_t const evtSize)
{
    uint8_t const poolNum = QF_priv_.maxPool_;

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the maximum of initialized pools so far must be in the configured range
    Q_REQUIRE_INCRIT(100, poolNum < QF_MAX_EPOOL);

    if (poolNum > 0U) { // any event pools already initialized?
        // the last initialized event pool must have event size smaller
        // than the one just being initialized
        // NOTE: QF event pools must be initialized in the increasing order
        // of their event sizes
        Q_REQUIRE_INCRIT(110,
            QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[poolNum - 1U]) < evtSize);
    }
    QF_priv_.maxPool_ = poolNum + 1U; // one more pool

    QF_CRIT_EXIT();

    // perform the port-dependent initialization of the event-pool
    QF_EPOOL_INIT_(QF_priv_.ePool_[poolNum], poolSto, poolSize, evtSize);

#ifdef Q_SPY
    // generate the QS object-dictionary entry for the initialized pool
    {
        uint8_t obj_name[9] = "EvtPool?"; // initial event pool name
        // replace the "?" with the one-digit pool number (1-based)
        obj_name[7] = (uint8_t)((uint8_t)'0' + QF_priv_.maxPool_);
        QS_obj_dict_pre_(&QF_priv_.ePool_[poolNum], (char const *)obj_name);
    }
#endif // Q_SPY
}

//............................................................................
//! @static @public @memberof QF
uint16_t QF_poolGetMaxBlockSize(void) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    uint8_t const maxPool = QF_priv_.maxPool_;

    // the maximum number of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(210, (0U < maxPool) && (maxPool <= QF_MAX_EPOOL));

    // set event size from the port-dependent operation
    uint16_t const maxSize =
        QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[maxPool - 1U]);
    QF_CRIT_EXIT();

    return maxSize;
}

//............................................................................
#ifdef QF_EPOOL_USE_
//! @static @public @memberof QF
uint16_t QF_getPoolUse(uint_fast8_t const poolNum) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    uint8_t const maxPool = QF_priv_.maxPool_;

    // the maximum number of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(310, maxPool <= QF_MAX_EPOOL);

    // the queried poolNum must be one of the initialized pools or 0
    Q_REQUIRE_INCRIT(320, poolNum <= maxPool);
#endif
    uint16_t nUse = 0U;
    if (poolNum > 0U) { // event pool number provided?
        // set event pool use from the port-dependent operation
        nUse = QF_EPOOL_USE_(&QF_priv_.ePool_[poolNum - 1U]);
    }
    else { // special case of poolNum==0
        // calculate the sum of used entries in all event pools
        for (uint_fast8_t pool = QF_priv_.maxPool_; pool > 0U; --pool) {
            // add the event pool use from the port-dependent operation
            nUse += QF_EPOOL_USE_(&QF_priv_.ePool_[pool - 1U]);
        }
    }

    QF_CRIT_EXIT();

    return nUse;
}
#endif // QF_EPOOL_USE_

//............................................................................
#ifdef QF_EPOOL_FREE_
//! @static @public @memberof QF
uint16_t QF_getPoolFree(uint_fast8_t const poolNum) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    uint8_t const maxPool = QF_priv_.maxPool_;

    // the maximum count of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(410, maxPool <= QF_MAX_EPOOL);

    // the poolNum paramter must be in range
    Q_REQUIRE_INCRIT(420, (0U < poolNum) && (poolNum <= maxPool));
#endif
    uint16_t const nFree = QF_EPOOL_FREE_(&QF_priv_.ePool_[poolNum - 1U]);

    QF_CRIT_EXIT();

    return nFree;
}
#endif // QF_EPOOL_FREE_

//............................................................................
#ifdef QF_EPOOL_MIN_
//! @static @public @memberof QF
uint16_t QF_getPoolMin(uint_fast8_t const poolNum) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    uint8_t const maxPool = QF_priv_.maxPool_;

    // the maximum count of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(510, maxPool <= QF_MAX_EPOOL);

    // the poolNum paramter must be in range
    Q_REQUIRE_INCRIT(520, (0U < poolNum) && (poolNum <= maxPool));
#endif
    // call port-specific operation for the minimum of free blocks so far
    uint16_t const nMin = QF_EPOOL_MIN_(&QF_priv_.ePool_[poolNum - 1U]);

    QF_CRIT_EXIT();

    return nMin;
}
#endif // QF_EPOOL_MIN_

//............................................................................
//! @static @private @memberof QF
QEvt * QF_newX_(
    uint_fast16_t const evtSize,
    uint_fast16_t const margin,
    enum_t const sig)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    uint8_t const maxPool = QF_priv_.maxPool_;

    // the maximum count of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(610, maxPool <= QF_MAX_EPOOL);

    // find the pool that fits the requested event size...
    uint8_t poolNum = 0U; // zero-based poolNum initially
    for (; poolNum < maxPool; ++poolNum) {
        // call port-specific operation for the event-size in a given pool
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[poolNum])) {
            break; // event pool found
        }
    }

    // event pool must be found, which means that the reqeusted event size
    // fits in one of the initialized pools
    Q_ASSERT_INCRIT(620, poolNum < maxPool);

    ++poolNum; // convert to 1-based poolNum

    QF_CRIT_EXIT();

    // get event e (port-dependent)...
    QEvt *e;
#ifdef Q_SPY
    QF_EPOOL_GET_(QF_priv_.ePool_[poolNum - 1U], e,
                  ((margin != QF_NO_MARGIN) ? margin : 0U),
                  QS_ID_EP + poolNum);
#else
    QF_EPOOL_GET_(QF_priv_.ePool_[poolNum - 1U], e,
                  ((margin != QF_NO_MARGIN) ? margin : 0U), 0U);
#endif

    if (e != (QEvt *)0) { // was e allocated correctly?
        e->sig      = (QSignal)sig; // set the signal
        e->poolNum_ = poolNum;
        e->refCtr_  = 0U; // reference count starts at 0

        QS_CRIT_ENTRY();
        QS_BEGIN_PRE(QS_QF_NEW, QS_ID_EP + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()
        QS_CRIT_EXIT();
    }
    else { // event was not allocated

        QF_CRIT_ENTRY();
        // This assertion means that the event allocation failed,
        // and this failure cannot be tolerated. The most frequent
        // reason is an event leak in the application.
        Q_ASSERT_INCRIT(630, margin != QF_NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_NEW_ATTEMPT, QS_ID_EP + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()

        QF_CRIT_EXIT();
    }

    // if we can't tolerate failed allocation (margin != QF_NO_MARGIN),
    // the returned event e is guaranteed to be valid (not NULL).
    return e;
}

//............................................................................
//! @static @public @memberof QF
void QF_gc(QEvt const * const e) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the collected event must be valid
    Q_REQUIRE_INCRIT(700, e != (QEvt *)0);

    uint8_t const poolNum = (uint8_t)e->poolNum_;
    if (poolNum != 0U) { // is it a pool event (mutable)?

        if (e->refCtr_ > 1U) { // isn't this the last reference?

            QS_BEGIN_PRE(QS_QF_GC_ATTEMPT, QS_ID_EP + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            QEvt_refCtr_dec_(e); // decrement the ref counter

            QF_CRIT_EXIT();
        }
        else { // this is the last reference to this event, recycle it
#ifndef Q_UNSAFE
            uint8_t const maxPool = QF_priv_.maxPool_;

            // the maximum count of initialized pools must be in configured range
            Q_ASSERT_INCRIT(740, maxPool <= QF_MAX_EPOOL);

            // the event poolNum must be one one the initialized event pools
            Q_ASSERT_INCRIT(750, poolNum <= maxPool);
#endif
            QS_BEGIN_PRE(QS_QF_GC, QS_ID_EP + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            QF_CRIT_EXIT();

            // call port-specific operation to put the event to a given pool
            // NOTE: casting 'const' away is legit because 'e' is a pool event
#ifdef Q_SPY
            QF_EPOOL_PUT_(QF_priv_.ePool_[poolNum - 1U], (QEvt *)e,
                QS_ID_EP + poolNum);
#else
            QF_EPOOL_PUT_(QF_priv_.ePool_[poolNum - 1U], (QEvt *)e, 0U);
#endif
        }
    }
    else {
        QF_CRIT_EXIT();
    }
}

//............................................................................
//! @static @private @memberof QF
QEvt const * QF_newRef_(
    QEvt const * const e,
    void const * const evtRef)
{
#ifdef Q_UNSAFE
    Q_UNUSED_PAR(evtRef);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the referenced event must be valid
    Q_REQUIRE_INCRIT(800, e != (QEvt *)0);

    // the event reference count must not exceed the number of AOs
    // in the system plus each AO possibly holding one event reference
    Q_REQUIRE_INCRIT(820, e->refCtr_ < (QF_MAX_ACTIVE + QF_MAX_ACTIVE));

    // the event ref must be valid
    Q_REQUIRE_INCRIT(830, evtRef == (void *)0);

    uint8_t const poolNum = (uint8_t)e->poolNum_;

    // the referenced event must be a pool event (not an immutable event)
    Q_ASSERT_INCRIT(840, poolNum != 0U);

    QEvt_refCtr_inc_(e); // increments the ref counter

    QS_BEGIN_PRE(QS_QF_NEW_REF, QS_ID_EP + poolNum)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of the event
        QS_2U8_PRE(poolNum, e->refCtr_);
    QS_END_PRE()

    QF_CRIT_EXIT();
    Q_UNUSED_PAR(poolNum); // might be unused

    return e;
}

//............................................................................
//! @static @private @memberof QF
void QF_deleteRef_(void const * const evtRef) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QEvt const * const e = (QEvt const *)evtRef;

    // the referenced event must be valid
    Q_REQUIRE_INCRIT(900, e != (QEvt *)0);

#ifdef Q_SPY
    uint8_t const poolNum = (uint8_t)e->poolNum_;

    QS_BEGIN_PRE(QS_QF_DELETE_REF, QS_ID_EP + poolNum)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of the event
        QS_2U8_PRE(poolNum, e->refCtr_);
    QS_END_PRE()
#endif // def Q_SPY

    QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
    QF_gc(e); // recycle the referenced event
#endif
}

#endif // (QF_MAX_EPOOL > 0U) mutable events configured
