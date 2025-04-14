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

Q_DEFINE_THIS_MODULE("qf_qeq")

//............................................................................
//! @public @memberof QEQueue
void QEQueue_init(QEQueue * const me,
    struct QEvt const  * * const qSto,
    uint_fast16_t const qLen)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#if (QF_EQUEUE_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(100, qLen < 0xFFU);
#endif

    me->frontEvt = (QEvt *)0; // no events in the queue
    me->ring     = qSto;      // the beginning of the ring buffer
    me->end      = (QEQueueCtr)qLen;
    if (qLen > 0U) {
        me->head = 0U;
        me->tail = 0U;
    }
    me->nFree    = (QEQueueCtr)(qLen + 1U); // +1 for frontEvt
    me->nMin     = me->nFree;

    QF_CRIT_EXIT();
}

//............................................................................
//! @public @memberof QEQueue
bool QEQueue_post(QEQueue * const me,
    struct QEvt const * const e,
    uint_fast16_t const margin,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

    QEQueueCtr tmp = me->nFree; // get volatile into temporary

    // can the queue accept the event?
    bool const status = ((margin == QF_NO_MARGIN) && (tmp > 0U))
        || (tmp > (QEQueueCtr)margin);
    if (status) {
        // is it a mutable event?
        if (e->poolNum_ != 0U) {
            Q_ASSERT_INCRIT(205, e->refCtr_ < (2U * QF_MAX_ACTIVE));
            QEvt_refCtr_inc_(e); // increment the reference counter
        }

        --tmp; // one free entry just used up

        me->nFree = tmp; // update the original
        if (me->nMin > tmp) { // is this the new minimum?
            me->nMin = tmp; // update minimum so far
        }

#ifdef Q_SPY
        QS_BEGIN_PRE(QS_QF_EQUEUE_POST, qsId)
            QS_TIME_PRE();        // timestamp
            QS_SIG_PRE(e->sig);   // the signal of the event
            QS_OBJ_PRE(me);       // this queue object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(tmp);      // # free entries
            QS_EQC_PRE(me->nMin); // min # free entries
        QS_END_PRE()
#endif // def Q_SPY

        if (me->frontEvt == (QEvt *)0) { // is the queue empty?
            me->frontEvt = e; // deliver event directly
        }
        else { // queue was not empty, insert event into the ring-buffer
            tmp = me->head; // get volatile into temporary
            me->ring[tmp] = e; // insert e into buffer

            if (tmp == 0U) { // need to wrap the head?
                tmp = me->end;
            }
            --tmp; // advance head (counter-clockwise)

            me->head = tmp; // update the original
        }
    }
    else { // event cannot be posted
        // dropping events must be acceptable
        Q_ASSERT_INCRIT(230, margin != QF_NO_MARGIN);

#ifdef Q_SPY
        QS_BEGIN_PRE(QS_QF_EQUEUE_POST_ATTEMPT, qsId)
            QS_TIME_PRE();       // timestamp
            QS_SIG_PRE(e->sig);  // the signal of this event
            QS_OBJ_PRE(me);      // this queue object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(tmp);     // # free entries
            QS_EQC_PRE(margin);  // margin requested
        QS_END_PRE()
#endif // def Q_SPY
    }

    QF_CRIT_EXIT();

    return status;
}

//............................................................................
//! @public @memberof QEQueue
void QEQueue_postLIFO(QEQueue * const me,
    struct QEvt const * const e,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(300, e != (QEvt *)0);

    QEQueueCtr tmp = me->nFree; // get volatile into temporary

    // must be able to LIFO-post the event
    Q_REQUIRE_INCRIT(310, tmp != 0U);

    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(305, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    --tmp; // one free entry just used up

    me->nFree = tmp; // update the original
    if (me->nMin > tmp) { // is this the new minimum?
        me->nMin = tmp; // update minimum so far
    }

    QS_BEGIN_PRE(QS_QF_EQUEUE_POST_LIFO, qsId)
        QS_TIME_PRE();        // timestamp
        QS_SIG_PRE(e->sig);   // the signal of this event
        QS_OBJ_PRE(me);       // this queue object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(tmp);      // # free entries
        QS_EQC_PRE(me->nMin); // min # free entries
    QS_END_PRE()

    QEvt const * const frontEvt = me->frontEvt;
    me->frontEvt = e; // deliver the event directly to the front

    if (frontEvt != (QEvt *)0) { // was the queue NOT empty?
        tmp = me->tail; // get volatile into temporary
        ++tmp;
        if (tmp == me->end) { // need to wrap the tail?
            tmp = 0U; // wrap around
        }
        me->tail = tmp;
        me->ring[tmp] = frontEvt;
    }

    QF_CRIT_EXIT();
}

//............................................................................
//! @public @memberof QEQueue
struct QEvt const * QEQueue_get(QEQueue * const me,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QEvt const * const e = me->frontEvt; // always remove evt from the front

    if (e != (QEvt *)0) { // was the queue not empty?
        QEQueueCtr tmp = me->nFree; // get volatile into temporary

        ++tmp; // one more free event in the queue

        me->nFree = tmp; // update the # free

        // any events in the ring buffer?
        if (tmp <= me->end) {

            QS_BEGIN_PRE(QS_QF_EQUEUE_GET, qsId)
                QS_TIME_PRE();      // timestamp
                QS_SIG_PRE(e->sig); // the signal of this event
                QS_OBJ_PRE(me);     // this queue object
                QS_2U8_PRE(e->poolNum_, e->refCtr_);
                QS_EQC_PRE(tmp);    // # free entries
            QS_END_PRE()

            tmp = me->tail; // get volatile into temporary
            QEvt const * const frontEvt = me->ring[tmp];

            Q_ASSERT_INCRIT(420, frontEvt != (QEvt *)0);

            me->frontEvt = frontEvt; // update the original

            if (tmp == 0U) { // need to wrap the tail?
                tmp = me->end;
            }
            --tmp; // advance the tail (counter-clockwise)
            me->tail = tmp; // update the original
        }
        else {
            me->frontEvt = (QEvt *)0; // queue becomes empty

            // all entries in the queue must be free (+1 for frontEvt)
            Q_ASSERT_INCRIT(440, tmp == (me->end + 1U));

            QS_BEGIN_PRE(QS_QF_EQUEUE_GET_LAST, qsId)
                QS_TIME_PRE();      // timestamp
                QS_SIG_PRE(e->sig); // the signal of this event
                QS_OBJ_PRE(me);     // this queue object
                QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_END_PRE()
        }
    }

    QF_CRIT_EXIT();

    return e;
}
