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

Q_DEFINE_THIS_MODULE("qf_actq")

//............................................................................
static void QActive_postFIFO_(QActive * const me,
    QEvt const * const e,
    void const * const sender);

//............................................................................
//! @private @memberof QActive
bool QActive_post_(QActive * const me,
    QEvt const * const e,
    uint_fast16_t const margin,
    void const * const sender)
{
#ifdef Q_UTEST // test?
#if (Q_UTEST != 0) // testing QP-stub?
    if (me->super.temp.fun == Q_STATE_CAST(0)) { // QActiveDummy?
        return QActiveDummy_fakePost_(me, e, margin, sender);
    }
#endif // (Q_UTEST != 0)
#endif // def Q_UTEST

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

    QEQueueCtr const nFree = me->eQueue.nFree; // get volatile into temporary

    bool status = (nFree > 0U);
    if (margin == QF_NO_MARGIN) { // no margin requested?
        // queue must not overflow
        Q_ASSERT_INCRIT(230, status);
    }
    else {
        status = (nFree > (QEQueueCtr)margin);
    }

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(240, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    if (status) { // can post the event?
        QActive_postFIFO_(me, e, sender);
#ifdef Q_UTEST
        if (QS_LOC_CHECK_(me->prio)) {
            QF_CRIT_EXIT();
            QS_onTestPost(sender, me, e, true); // QUTest callback
            QF_CRIT_ENTRY();
        }
#endif // def Q_UTEST
        QF_CRIT_EXIT();
    }
    else { // event cannot be posted
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, me->prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(me);      // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries
            QS_EQC_PRE(margin);  // margin requested
        QS_END_PRE()

#ifdef Q_UTEST
        if (QS_LOC_CHECK_(me->prio)) {
            QF_CRIT_EXIT();
            QS_onTestPost(sender, me, e, status); // QUTEst callback
            QF_CRIT_ENTRY();
        }
#endif // def Q_USTEST

        QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
        QF_gc(e); // recycle the event to avoid a leak
#endif // (QF_MAX_EPOOL > 0U)
    }

    return status;
}

//............................................................................
//! @private @memberof QActive
void QActive_postLIFO_(QActive * const me,
    QEvt const * const e)
{
#ifdef Q_UTEST // test?
#if (Q_UTEST != 0) // testing QP-stub?
    if (me->super.temp.fun == Q_STATE_CAST(0)) { // QActiveDummy?
        QActiveDummy_fakePostLIFO_(me, e);
        return;
    }
#endif // (Q_UTEST != 0)
#endif // def Q_UTEST

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the posted event must be be valid (which includes not NULL)
    Q_REQUIRE_INCRIT(300, e != (QEvt *)0);

    QEQueueCtr nFree = me->eQueue.nFree; // get volatile into temporary

    // The queue must NOT overflow for the LIFO posting policy.
    Q_REQUIRE_INCRIT(330, nFree != 0U);

    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(340, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    --nFree; // one free entry just used up
    me->eQueue.nFree = nFree; // update the original
    if (me->eQueue.nMin > nFree) {
        me->eQueue.nMin = nFree; // update minimum so far
    }

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(nFree);   // # free entries
        QS_EQC_PRE(me->eQueue.nMin); // min # free entries
    QS_END_PRE()

#ifdef Q_UTEST
    // callback to examine the posted event under the same conditions
    // as producing the #QS_QF_ACTIVE_POST trace record, which are:
    // the local filter for this AO ('me->prio') is set
    if (QS_LOC_CHECK_(me->prio)) {
        QF_CRIT_EXIT();
        QS_onTestPost((QActive *)0, me, e, true);
        QF_CRIT_ENTRY();
    }
#endif // def Q_UTEST

    QEvt const * const frontEvt = me->eQueue.frontEvt;
    me->eQueue.frontEvt = e; // deliver the event directly to the front

    if (frontEvt != (QEvt *)0) { // was the queue NOT empty?
        QEQueueCtr tail = me->eQueue.tail; // get volatile into temporary
        ++tail;
        if (tail == me->eQueue.end) { // need to wrap the tail?
            tail = 0U; // wrap around
        }
        me->eQueue.tail = tail;
        me->eQueue.ring[tail] = frontEvt;
    }
    else { // queue was empty
        QACTIVE_EQUEUE_SIGNAL_(me); // signal the event queue
    }

    QF_CRIT_EXIT();
}

//............................................................................
//! @private @memberof QActive
QEvt const * QActive_get_(QActive * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // wait for event to arrive directly (depends on QP port)
    // NOTE: might use assertion-IDs 400-409
    QACTIVE_EQUEUE_WAIT_(me);

    // always remove event from the front
    QEvt const * const e = me->eQueue.frontEvt;
    Q_REQUIRE_INCRIT(410, e != (QEvt *)0); // queue must NOT be empty

    QEQueueCtr nFree = me->eQueue.nFree; // get volatile into temporary

    ++nFree; // one more free event in the queue
    me->eQueue.nFree = nFree; // update the # free

    if (nFree <= me->eQueue.end) { // any events in the ring buffer?

        // remove event from the tail
        QEQueueCtr tail = me->eQueue.tail; // get volatile into temporary

        QEvt const * const frontEvt = me->eQueue.ring[tail];
        Q_ASSERT_INCRIT(450, frontEvt != (QEvt *)0);

        QS_BEGIN_PRE(QS_QF_ACTIVE_GET, me->prio)
            QS_TIME_PRE();       // timestamp
            QS_SIG_PRE(e->sig);  // the signal of this event
            QS_OBJ_PRE(me);      // this active object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries
        QS_END_PRE()

        me->eQueue.frontEvt = frontEvt; // update the original
        if (tail == 0U) { // need to wrap the tail?
            tail = me->eQueue.end;
        }
        --tail; // advance the tail (counter-clockwise)
        me->eQueue.tail = tail; // update the original
    }
    else {
        me->eQueue.frontEvt = (QEvt *)0; // queue becomes empty

        // all entries in the queue must be free (+1 for fronEvt)
        Q_ASSERT_INCRIT(460, nFree == (me->eQueue.end + 1U));

        QS_BEGIN_PRE(QS_QF_ACTIVE_GET_LAST, me->prio)
            QS_TIME_PRE();       // timestamp
            QS_SIG_PRE(e->sig);  // the signal of this event
            QS_OBJ_PRE(me);      // this active object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_END_PRE()
    }

    QF_CRIT_EXIT();

    return e;
}

//............................................................................
//! @private @memberof QActive
static void QActive_postFIFO_(QActive * const me,
    QEvt const * const e,
    void const * const sender)
{
    // NOTE: this helper function is called *inside* critical section
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QEQueueCtr nFree = me->eQueue.nFree; // get volatile into temporary

    --nFree; // one free entry just used up
    me->eQueue.nFree = nFree; // update the original
    if (me->eQueue.nMin > nFree) {
        me->eQueue.nMin = nFree; // update minimum so far
    }

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST, me->prio)
        QS_TIME_PRE();        // timestamp
        QS_OBJ_PRE(sender);   // the sender object
        QS_SIG_PRE(e->sig);   // the signal of the event
        QS_OBJ_PRE(me);       // this active object (recipient)
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(nFree);    // # free entries
        QS_EQC_PRE(me->eQueue.nMin); // min # free entries
    QS_END_PRE()

    if (me->eQueue.frontEvt == (QEvt *)0) { // is the queue empty?
        me->eQueue.frontEvt = e; // deliver event directly

#ifdef QXK_H_
        if (me->super.state.act == Q_ACTION_CAST(0)) { // eXtended thread?
            QXTHREAD_EQUEUE_SIGNAL_(me); // signal eXtended Thread
        }
        else {
            QACTIVE_EQUEUE_SIGNAL_(me); // signal the Active Object
        }
#else
        QACTIVE_EQUEUE_SIGNAL_(me); // signal the Active Object
#endif // def QXK_H_
    }
    else { // queue was not empty, insert event into the ring-buffer
        QEQueueCtr head = me->eQueue.head; // get volatile into temporary
        me->eQueue.ring[head] = e; // insert e into buffer

        if (head == 0U) { // need to wrap the head?
            head = me->eQueue.end;
        }
        --head; // advance the head (counter-clockwise)

        me->eQueue.head = head; // update the original
    }
}

//............................................................................
//! @static @public @memberof QActive
uint_fast16_t QActive_getQueueMin(uint_fast8_t const prio) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(600, prio <= QF_MAX_ACTIVE);

    QActive const * const a = QActive_registry_[prio];
    Q_REQUIRE_INCRIT(610, a != (QActive *)0);

    uint_fast16_t const min = (uint_fast16_t)(a->eQueue.nMin);
    QF_CRIT_EXIT();

    return min;
}

//============================================================================
//! @public @memberof QTicker
void QTicker_ctor(QTicker * const me,
    uint_fast8_t const tickRate)
{
    QActive_ctor(&me->super, Q_STATE_CAST(0)); // superclass' ctor

    static struct QAsmVtable const vtable = { // QTicker virtual table
        &QTicker_init_,
        &QTicker_dispatch_,
        &QHsm_isIn_
#ifdef Q_SPY
        ,&QHsm_getStateHandler_
#endif
    };
    me->super.super.vptr = &vtable; // hook the vptr

    // reuse eQueue.head for tick-rate
    me->super.eQueue.head = (QEQueueCtr)tickRate;
}

//............................................................................
//! @private @memberof QTicker
void QTicker_init_(
    QAsm * const me,
    void const * const par,
    uint_fast8_t const qsId)
{
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(par);
    Q_UNUSED_PAR(qsId);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QACTIVE_CAST_(me)->eQueue.tail = 0U;

    QF_CRIT_EXIT();
}

//............................................................................
//! @private @memberof QTicker
void QTicker_dispatch_(
    QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qsId);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // get volatile into temporaries
    QEQueueCtr nTicks = QACTIVE_CAST_(me)->eQueue.tail;
    QEQueueCtr const tickRate = QACTIVE_CAST_(me)->eQueue.head;

    Q_REQUIRE_INCRIT(800, nTicks > 0U);

    QACTIVE_CAST_(me)->eQueue.tail = 0U; // clear # ticks

    QF_CRIT_EXIT();

    for (; nTicks > 0U; --nTicks) {
        QTimeEvt_tick_((uint_fast8_t)tickRate, me);
    }
}

//............................................................................
//! @private @memberof QTicker
void QTicker_trig_(
    QTicker * const me,
    void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    static QEvt const tickEvt = QEVT_INITIALIZER(0);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QEQueueCtr nTicks = me->super.eQueue.tail; // get volatile into temporary

    if (me->super.eQueue.frontEvt == (QEvt *)0) { // no tick events?
        Q_REQUIRE_INCRIT(930, me->super.eQueue.nFree == 1U);
        Q_REQUIRE_INCRIT(940, nTicks == 0U);

        me->super.eQueue.frontEvt = &tickEvt; // deliver event directly
        me->super.eQueue.nFree = 0U;

        QACTIVE_EQUEUE_SIGNAL_(&me->super); // signal the event queue
    }
    else {
        Q_REQUIRE_INCRIT(950, (0U < nTicks) && (nTicks < 0xFFU));
    }

    ++nTicks; // account for one more tick event
    me->super.eQueue.tail = nTicks; // update the original

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST, me->super.prio)
        QS_TIME_PRE();      // timestamp
        QS_OBJ_PRE(sender); // the sender object
        QS_SIG_PRE(0U);     // the signal of the event
        QS_OBJ_PRE(me);     // this active object
        QS_2U8_PRE(0U, 0U); // poolNum & refCtr
        QS_EQC_PRE(0U);     // # free entries
        QS_EQC_PRE(0U);     // min # free entries
    QS_END_PRE()

    QF_CRIT_EXIT();
}
