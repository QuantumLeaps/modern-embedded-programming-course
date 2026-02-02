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

//============================================================================
#if (QF_MAX_TICK_RATE > 0U)

Q_DEFINE_THIS_MODULE("qf_time")

//............................................................................
QTimeEvt QTimeEvt_timeEvtHead_[QF_MAX_TICK_RATE];

//............................................................................
//! @public @memberof QTimeEvt
void QTimeEvt_ctorX(QTimeEvt * const me,
    QActive * const act,
    enum_t const sig,
    uint_fast8_t const tickRate)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the signal must be != 0, but other reserved signals are allowed
    Q_REQUIRE_INCRIT(300, sig != 0);

    // the tick rate must be in the configured range
    Q_REQUIRE_INCRIT(310, tickRate < QF_MAX_TICK_RATE);
    QF_CRIT_EXIT();

    QEvt_ctor(&me->super, sig); // the superclass' ctor

    me->super.refCtr_ = 0U; // adjust from QEvt_ctor(sig)
    me->next     = (QTimeEvt *)0;
    me->act      = act; // might be NULL for a time-event head
    me->ctr      = 0U;
    me->interval = 0U;
    me->tickRate = (uint8_t)tickRate;
    me->flags    = 0U;
}

//............................................................................
//! @public @memberof QTimeEvt
void QTimeEvt_armX(QTimeEvt * const me,
    uint32_t const nTicks,
    uint32_t const interval)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // nTicks and interval parameters must fit in the configured dynamic range
#if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(400, nTicks   < 0xFFU);
    Q_REQUIRE_INCRIT(410, interval < 0xFFU);
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(400, nTicks   < 0xFFFFU);
    Q_REQUIRE_INCRIT(410, interval < 0xFFFFU);
#endif

#ifndef Q_UNSAFE
    QTimeEvtCtr const ctr = me->ctr;
#endif

    uint8_t const tickRate = me->tickRate;

    // nTicks must be != 0 for arming a time event
    Q_REQUIRE_INCRIT(440, nTicks != 0U);

    // the time event must not be already armed
    Q_REQUIRE_INCRIT(450, ctr == 0U);

    // the AO associated with this time event must be valid
    Q_REQUIRE_INCRIT(460, me->act != (void *)0);

    // the tick rate of this time event must be in range
    Q_REQUIRE_INCRIT(470, tickRate < QF_MAX_TICK_RATE);

    me->ctr = (QTimeEvtCtr)nTicks;
    me->interval = (QTimeEvtCtr)interval;

    // is the time event unlinked?
    // NOTE: For the duration of a single clock tick of the specified tick
    // rate a time event can be disarmed and yet still linked into the list
    // because un-linking is performed exclusively in QTimeEvt_tick_().
    if ((me->flags & QTE_FLAG_IS_LINKED) == 0U) {
        // The time event is initially inserted into the separate
        // "freshly armed" list based on timeEvtHead_[tickRate].act.
        // Only later, inside QTimeEvt_tick_(), the "freshly armed"
        // list is appended to the main list of armed time events based on
        // timeEvtHead_[tickRate].next. Again, this is to keep any
        // changes to the main list exclusively inside QTimeEvt_tick_().

        me->flags |= QTE_FLAG_IS_LINKED; // mark as linked
        me->next = (QTimeEvt *)QTimeEvt_timeEvtHead_[tickRate].act;
        QTimeEvt_timeEvtHead_[tickRate].act = me;
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_ARM, ((QActive *)(me->act))->prio)
        QS_TIME_PRE();        // timestamp
        QS_OBJ_PRE(me);       // this time event object
        QS_OBJ_PRE(me->act);  // the active object
        QS_TEC_PRE(nTicks);   // the # ticks
        QS_TEC_PRE(interval); // the interval
        QS_U8_PRE(tickRate);  // tick rate
    QS_END_PRE()

    QF_CRIT_EXIT();
}

//............................................................................
//! @public @memberof QTimeEvt
bool QTimeEvt_disarm(QTimeEvt * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QTimeEvtCtr const ctr = me->ctr; // move member into temporary

#ifdef Q_SPY
    uint_fast8_t const qsId = QACTIVE_CAST_(me->act)->prio;
#endif

    // was the time event actually armed?
    bool wasArmed = false;
    if (ctr != 0U) {
        wasArmed = true;
        me->flags |= QTE_FLAG_WAS_DISARMED;
        me->ctr = 0U; // schedule removal from the list

        QS_BEGIN_PRE(QS_QF_TIMEEVT_DISARM, qsId)
            QS_TIME_PRE();            // timestamp
            QS_OBJ_PRE(me);           // this time event object
            QS_OBJ_PRE(me->act);      // the target AO
            QS_TEC_PRE(ctr);          // the # ticks
            QS_TEC_PRE(me->interval); // the interval
            QS_U8_PRE(me->tickRate);  // tick rate
        QS_END_PRE()
    }
    else { // the time event was already disarmed automatically
        me->flags &= (uint8_t)(~QTE_FLAG_WAS_DISARMED & 0xFFU);

        QS_BEGIN_PRE(QS_QF_TIMEEVT_DISARM_ATTEMPT, qsId)
            QS_TIME_PRE();            // timestamp
            QS_OBJ_PRE(me);           // this time event object
            QS_OBJ_PRE(me->act);      // the target AO
            QS_U8_PRE(me->tickRate);  // tick rate
        QS_END_PRE()
    }

    QF_CRIT_EXIT();

    return wasArmed;
}

//............................................................................
//! @public @memberof QTimeEvt
bool QTimeEvt_rearm(QTimeEvt * const me, uint32_t const nTicks) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // nTicks parameter must fit in the configured dynamic range
#if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFU);
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFFFU);
#endif

    uint8_t const tickRate = me->tickRate;
    QTimeEvtCtr const ctr = me->ctr;

    // nTicks must be != 0 for arming a time event
    Q_REQUIRE_INCRIT(610, nTicks != 0U);

    // the AO associated with this time event must be valid
    Q_REQUIRE_INCRIT(620, me->act != (void *)0);

    // the tick rate of this time event must be in range
    Q_REQUIRE_INCRIT(630, tickRate < QF_MAX_TICK_RATE);

#ifdef Q_SPY
    uint_fast8_t const qsId = ((QActive *)(me->act))->prio;
#endif

    me->ctr = (QTimeEvtCtr)nTicks;

    // was the time evt not running?
    bool wasArmed = false;
    if (ctr == 0U) {
        // NOTE: For the duration of a single clock tick of the specified
        // tick rate a time event can be disarmed and yet still linked into
        // the list, because unlinking is performed exclusively in the
        // QTimeEvt_tick_() function.

        // is the time event unlinked?
        if ((me->flags & QTE_FLAG_IS_LINKED) == 0U) {

            // The time event is initially inserted into the separate
            // "freshly armed" list based on timeEvtHead_[tickRate].act.
            // Only later, inside QTimeEvt_tick_(), the "freshly armed"
            // list is appended to the main list of armed time events based on
            // timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QTimeEvt_tick()_.

            me->flags |= QTE_FLAG_IS_LINKED; // mark as linked
            me->next = (QTimeEvt *)QTimeEvt_timeEvtHead_[tickRate].act;
            QTimeEvt_timeEvtHead_[tickRate].act = me;
        }
    }
    else { // the time event was armed
        wasArmed = true;
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_REARM, qsId)
        QS_TIME_PRE();            // timestamp
        QS_OBJ_PRE(me);           // this time event object
        QS_OBJ_PRE(me->act);      // the target AO
        QS_TEC_PRE(nTicks);       // the # ticks
        QS_TEC_PRE(me->interval); // the interval
        QS_2U8_PRE(tickRate, (wasArmed ? 1U : 0U));
    QS_END_PRE()

    QF_CRIT_EXIT();

    return wasArmed;
}

//............................................................................
//! @public @memberof QTimeEvt
bool QTimeEvt_wasDisarmed(QTimeEvt * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // was this time-event disarmed automatically upon expiration?
    bool const wasDisarmed = (me->flags & QTE_FLAG_WAS_DISARMED) != 0U;

    me->flags |= QTE_FLAG_WAS_DISARMED; // mark as disarmed (SIDE EFFECT!)

    QF_CRIT_EXIT();

    return wasDisarmed;
}

//............................................................................
//! @static @private @memberof QTimeEvt
void QTimeEvt_tick_(
    uint_fast8_t const tickRate,
    void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the tick rate of this time event must be in range
    Q_REQUIRE_INCRIT(800, tickRate < QF_MAX_TICK_RATE);

    QTimeEvt *prev = &QTimeEvt_timeEvtHead_[tickRate];

#ifdef Q_SPY
    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        ++prev->ctr;
        QS_TEC_PRE(prev->ctr);   // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()
#endif

    // scan the linked-list of time events at this tick rate...
    for (;;) {
        QTimeEvt *te = prev->next; // advance down the time evt. list

        if (te == (QTimeEvt *)0) { // end of the list?
            // set 'te' to the the newly-armed linked list
            te = (QTimeEvt *)QTimeEvt_timeEvtHead_[tickRate].act;
            if (te == (void *)0) { // no newly-armed time events?
                break; // terminate the loop
            }

            prev->next = te;
            QTimeEvt_timeEvtHead_[tickRate].act = (void *)0;
        }

        QTimeEvtCtr ctr = te->ctr; // move member into temporary

        if (ctr == 0U) { // time event scheduled for removal?
            prev->next = te->next;

            // mark time event 'te' as NOT linked
            te->flags &= (uint8_t)(~QTE_FLAG_IS_LINKED & 0xFFU);
            // do NOT advance the prev pointer
            QF_CRIT_EXIT(); // exit crit. section to reduce latency
        }
        else if (ctr == 1U) { // is time event about to expire?
            QActive * const act = (QActive *)te->act;
            prev = QTimeEvt_expire_(te, prev, act, tickRate);

#ifdef QXK_H_
            if ((enum_t)te->super.sig < Q_USER_SIG) {
                QXThread_timeout_(act);
                QF_CRIT_EXIT();
            }
            else {
                QF_CRIT_EXIT(); // exit crit. section before posting

                // QACTIVE_POST() asserts if the queue overflows
                QACTIVE_POST(act, &te->super, sender);
            }
#else // not QXK
            QF_CRIT_EXIT(); // exit crit. section before posting

            // QACTIVE_POST() asserts if the queue overflows
            QACTIVE_POST(act, &te->super, sender);
#endif
        }
        else { // time event keeps timing out
            --ctr; // decrement the tick counter
            te->ctr = ctr; // update the member original

            prev = te; // advance to this time event
            QF_CRIT_EXIT(); // exit crit. section to reduce latency
        }
        QF_CRIT_ENTRY(); // re-enter crit. section to continue the loop
    }
    QF_CRIT_EXIT();
}

//............................................................................
//! @static @public @memberof QTimeEvt
bool QTimeEvt_noActive(uint_fast8_t const tickRate) {
    // NOTE: this function must be called *inside* critical section
    Q_REQUIRE_INCRIT(900, tickRate < QF_MAX_TICK_RATE);

    QTimeEvt const * const head = &QTimeEvt_timeEvtHead_[tickRate];
    bool const noActive =
        (head->next == (QTimeEvt *)0) && (head->act == (void *)0);

    return noActive;
}

//............................................................................
//! @public @memberof QTimeEvt
QTimeEvtCtr QTimeEvt_getCtr(QTimeEvt const * const me) {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return me->ctr;
}

//............................................................................
//! @static @private @memberof QTimeEvt
void QTimeEvt_init(void) {
    // call ctors for time event heads for all configured tick rates
    for (uint_fast8_t tickRate = 0U;
         tickRate < Q_DIM(QTimeEvt_timeEvtHead_);
         ++tickRate)
    {
        // time event head has invalid AO and Q_USER_SIG as signal
        QTimeEvt_ctorX(&QTimeEvt_timeEvtHead_[tickRate],
                       (QActive *)0, Q_USER_SIG, tickRate);
    }
}

//............................................................................
//! @private @memberof QTimeEvt
QTimeEvt * QTimeEvt_expire_(QTimeEvt * const me,
    QTimeEvt * const prev_link,
    QActive const * const act,
    uint_fast8_t const tickRate)
{
    // NOTE: this helper function is called *inside* critical section
#ifndef Q_SPY
    Q_UNUSED_PAR(act);
    Q_UNUSED_PAR(tickRate);
#endif

    QTimeEvt *prev = prev_link;
    if (me->interval != 0U) { // periodic time evt?
        me->ctr = me->interval; // rearm the time event
        prev = me; // advance to this time event
    }
    else { // one-shot time event: automatically disarm
        me->ctr = 0U;
        prev->next = me->next;

        // mark this time event as NOT linked
        me->flags &= (uint8_t)(~QTE_FLAG_IS_LINKED & 0xFFU);
        // do NOT advance the prev pointer

        QS_BEGIN_PRE(QS_QF_TIMEEVT_AUTO_DISARM, act->prio)
            QS_OBJ_PRE(me);       // this time event object
            QS_OBJ_PRE(act);      // the target AO
            QS_U8_PRE(tickRate);  // tick rate
        QS_END_PRE()
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_POST, act->prio)
        QS_TIME_PRE();            // timestamp
        QS_OBJ_PRE(me);           // the time event object
        QS_SIG_PRE(me->super.sig);// signal of this time event
        QS_OBJ_PRE(act);          // the target AO
        QS_U8_PRE(tickRate);      // tick rate
    QS_END_PRE()

    return prev;
}

#else // (QF_MAX_TICK_RATE > 0U)

//............................................................................
void QTimeEvt_init(void) { // dummy init
}

#endif // (QF_MAX_TICK_RATE > 0U)
