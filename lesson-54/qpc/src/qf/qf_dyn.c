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
    uint_fast8_t const poolNum = QF_priv_.maxPool_;

    // see precondition{qf_dyn,200} and precondition{qf_dyn,201}
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(100, poolNum < QF_MAX_EPOOL);
    if (poolNum > 0U) {
        Q_REQUIRE_INCRIT(110,
            QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[poolNum - 1U]) < evtSize);
    }
    QF_priv_.maxPool_ = poolNum + 1U; // one more pool

    QF_CRIT_EXIT();

    // perform the port-dependent initialization of the event-pool
    QF_EPOOL_INIT_(QF_priv_.ePool_[poolNum], poolSto, poolSize, evtSize);

#ifdef Q_SPY
    // generate the object-dictionary entry for the initialized pool
    {
        uint8_t obj_name[9] = "EvtPool?";
        obj_name[7] = (uint8_t)((uint8_t)'0' + poolNum + 1U);
        QS_obj_dict_pre_(&QF_priv_.ePool_[poolNum], (char const *)obj_name);
    }
#endif // Q_SPY
}

//............................................................................
//! @static @public @memberof QF
uint_fast16_t QF_poolGetMaxBlockSize(void) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    uint_fast16_t const max_size =
        QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[QF_priv_.maxPool_ - 1U]);
    QF_CRIT_EXIT();

    return max_size;
}

//............................................................................
//! @static @public @memberof QF
uint_fast16_t QF_getPoolMin(uint_fast8_t const poolNum) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(300, (poolNum <= QF_MAX_EPOOL)
                      && (0U < poolNum) && (poolNum <= QF_priv_.maxPool_));

    uint_fast16_t const min = (uint_fast16_t)QF_priv_.ePool_[poolNum - 1U].nMin;

    QF_CRIT_EXIT();

    return min;
}

//............................................................................
//! @static @private @memberof QF
QEvt * QF_newX_(
    uint_fast16_t const evtSize,
    uint_fast16_t const margin,
    enum_t const sig)
{
    // find the pool id that fits the requested event size...
    uint8_t poolNum = 0U; // zero-based poolNum initially
    for (; poolNum < QF_priv_.maxPool_; ++poolNum) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[poolNum])) {
            break;
        }
    }

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // precondition:
    // - cannot run out of registered pools
    Q_REQUIRE_INCRIT(400, poolNum < QF_priv_.maxPool_);

    ++poolNum; // convert to 1-based poolNum

    QF_CRIT_EXIT();

    // get event e (port-dependent)...
    QEvt *e;
#ifdef Q_SPY
    QF_EPOOL_GET_(QF_priv_.ePool_[poolNum - 1U], e,
                  ((margin != QF_NO_MARGIN) ? margin : 0U),
                  (uint_fast8_t)QS_EP_ID + poolNum);
#else
    QF_EPOOL_GET_(QF_priv_.ePool_[poolNum - 1U], e,
                  ((margin != QF_NO_MARGIN) ? margin : 0U), 0U);
#endif

    if (e != (QEvt *)0) { // was e allocated correctly?
        e->sig      = (QSignal)sig; // set the signal
        e->poolNum_ = poolNum;
        e->refCtr_  = 0U;

        QS_CRIT_ENTRY();
        QS_BEGIN_PRE(QS_QF_NEW, (uint_fast8_t)QS_EP_ID + poolNum)
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
        Q_ASSERT_INCRIT(420, margin != QF_NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_NEW_ATTEMPT,
                (uint_fast8_t)QS_EP_ID + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()

        QF_CRIT_EXIT();
    }

    // the returned event e is guaranteed to be valid (not NULL)
    // if we can't tolerate failed allocation
    return e;
}

//............................................................................
//! @static @public @memberof QF
void QF_gc(QEvt const * const e) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(500, e != (QEvt *)0);

    uint8_t const poolNum = e->poolNum_;

    if (poolNum != 0U) { // is it a pool event (mutable)?

        if (e->refCtr_ > 1U) { // isn't this the last reference?

            QS_BEGIN_PRE(QS_QF_GC_ATTEMPT,
                    (uint_fast8_t)QS_EP_ID + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            Q_ASSERT_INCRIT(505, e->refCtr_ > 0U);
            QEvt_refCtr_dec_(e); // decrement the ref counter

            QF_CRIT_EXIT();
        }
        else { // this is the last reference to this event, recycle it

            QS_BEGIN_PRE(QS_QF_GC,
                    (uint_fast8_t)QS_EP_ID + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            // pool number must be in range
            Q_ASSERT_INCRIT(510, (poolNum <= QF_priv_.maxPool_)
                                  && (poolNum <= QF_MAX_EPOOL));
            QF_CRIT_EXIT();

            // NOTE: casting 'const' away is legit because 'e' is a pool event
#ifdef Q_SPY
            QF_EPOOL_PUT_(QF_priv_.ePool_[poolNum - 1U], (QEvt *)e,
                (uint_fast8_t)QS_EP_ID + poolNum);
#else
            QF_EPOOL_PUT_(QF_priv_.ePool_[poolNum - 1U], (QEvt *)e,
                0U);
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

    Q_REQUIRE_INCRIT(600, e != (QEvt *)0);

    uint_fast8_t const poolNum = e->poolNum_;
    Q_UNUSED_PAR(poolNum); // might be unused

    Q_REQUIRE_INCRIT(610, (poolNum != 0U)
        && (evtRef == (void *)0));

    Q_ASSERT_INCRIT(605, e->refCtr_ < (2U * QF_MAX_ACTIVE));
    QEvt_refCtr_inc_(e); // increments the ref counter

    QS_BEGIN_PRE(QS_QF_NEW_REF,
            (uint_fast8_t)QS_EP_ID + poolNum)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of the event
        QS_2U8_PRE(poolNum, e->refCtr_);
    QS_END_PRE()

    QF_CRIT_EXIT();

    return e;
}

//............................................................................
//! @static @private @memberof QF
void QF_deleteRef_(void const * const evtRef) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QEvt const * const e = (QEvt const *)evtRef;
    Q_REQUIRE_INCRIT(700, e != (QEvt *)0);

#ifdef Q_SPY
    uint_fast8_t const poolNum = e->poolNum_;
    QS_BEGIN_PRE(QS_QF_DELETE_REF,
            (uint_fast8_t)QS_EP_ID + poolNum)
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
