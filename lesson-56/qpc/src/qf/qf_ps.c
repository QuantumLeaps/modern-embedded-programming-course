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

Q_DEFINE_THIS_MODULE("qf_ps")

QSubscrList * QActive_subscrList_;
QSignal QActive_maxPubSignal_;


// static local helper function (declaration)
static void QActive_multicast_(
    QPSet * const subscrSet,
    QEvt const * const e,
    void const * const sender);

//............................................................................
//! @static @public @memberof QActive
void QActive_psInit(
    QSubscrList * const subscrSto,
    enum_t const maxSignal)
{
    // provided subscSto must be valid
    Q_REQUIRE_INCRIT(100, subscrSto != (QSubscrList *)0);

    // provided maximum of subscribed signals must be >= Q_USER_SIG
    Q_REQUIRE_INCRIT(110, maxSignal >= Q_USER_SIG);

    QActive_subscrList_   = subscrSto;
    QActive_maxPubSignal_ = (QSignal)maxSignal;

    // initialize all signals in the subscriber list...
    for (enum_t sig = 0; sig < maxSignal; ++sig) {
        QPSet_setEmpty(&subscrSto[sig].set); // no subscibers to this signal
    }
}

//............................................................................
//! @static @private @memberof QActive
void QActive_publish_(
    QEvt const * const e,
    void const * const sender,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the published event must be valid
    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

    QSignal const sig = (QSignal)e->sig;

    // published event signal must not exceed the maximum
    Q_REQUIRE_INCRIT(240, sig < QActive_maxPubSignal_);

    // make a local, modifiable copy of the subscriber set
    QPSet subscrSet = QActive_subscrList_[sig].set;

    QS_BEGIN_PRE(QS_QF_PUBLISH, qsId)
        QS_TIME_PRE();          // the timestamp
        QS_OBJ_PRE(sender);     // the sender object
        QS_SIG_PRE(sig);        // the signal of the event
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
    QS_END_PRE()

    if (e->poolNum_ != 0U) { // is it a mutable event?
        // NOTE: The reference counter of a mutable event is incremented to
        // prevent premature recycling of the event while multicasting is
        // still in progress. The garbage collector step (QF_gc()) at the
        // end of the function decrements the reference counter and recycles
        // the event if the counter drops to zero. This covers the case when
        // event was published without any subscribers.
        QEvt_refCtr_inc_(e);
    }

    QF_CRIT_EXIT();

    if (QPSet_notEmpty(&subscrSet)) { // any subscribers?
        QActive_multicast_(&subscrSet, e, sender); // multicast to all
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
#if (QF_MAX_EPOOL > 0U)
    QF_gc(e); // recycle the event to avoid a leak
#endif
}

//............................................................................
//! @private @memberof QActive
static void QActive_multicast_(
    QPSet * const subscrSet,
    QEvt const * const e,
    void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    // highest-prio subscriber ('subscrSet' guaranteed to be NOT empty)
    uint8_t p = (uint8_t)QPSet_findMax(subscrSet);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // p != 0 is guaranteed as the result of QPSet_findMax()
    Q_ASSERT_INCRIT(300, p <= QF_MAX_ACTIVE);
    QActive *a = QActive_registry_[p];

    // the active object must be registered (started)
    Q_ASSERT_INCRIT(310, a != (QActive *)0);

    QF_CRIT_EXIT();

    QF_SCHED_STAT_
    QF_SCHED_LOCK_(p); // lock the scheduler up to AO's prio

    // NOTE: the following loop does not need the fixed loop bound check
    // because the local subscriber set 'subscrSet' can hold at most
    // QF_MAX_ACTIVE elements (rounded up to the nearest 8), which are
    // removed one by one at every pass.
    for (;;) { // loop over all subscribers

        // QACTIVE_POST() asserts internally if the queue overflows
        QACTIVE_POST(a, e, sender);

        QPSet_remove(subscrSet, p); // remove the handled subscriber
        if (QPSet_isEmpty(subscrSet)) {  // no more subscribers?
            break;
        }

        // find the next highest-prio subscriber
        p = (uint8_t)QPSet_findMax(subscrSet);

        QF_CRIT_ENTRY();

        a = QActive_registry_[p];

        // the AO must be registered with the framework
        Q_ASSERT_INCRIT(340, a != (QActive *)0);

        QF_CRIT_EXIT();
    }

    QF_SCHED_UNLOCK_(); // unlock the scheduler
}

//............................................................................
//! @protected @memberof QActive
void QActive_subscribe(QActive const * const me,
    enum_t const sig)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    uint8_t const p = me->prio;

    // the AO's prio. must be in range
    Q_REQUIRE_INCRIT(420, (0U < p) && (p <= QF_MAX_ACTIVE));

    // the subscriber AO must be registered (started)
    Q_REQUIRE_INCRIT(440, me == QActive_registry_[p]);

    // the sig parameter must not overlap reserved signals
    Q_REQUIRE_INCRIT(460, sig >= Q_USER_SIG);

    // the subscribed signal must be below the maximum of published signals
    Q_REQUIRE_INCRIT(480, (QSignal)sig < QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_SUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(me);   // this active object
    QS_END_PRE()

    // insert the AO's prio. into the subscriber set for the signal
    QPSet_insert(&QActive_subscrList_[sig].set, p);

    QF_CRIT_EXIT();
}

//............................................................................
//! @protected @memberof QActive
void QActive_unsubscribe(QActive const * const me,
    enum_t const sig)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    uint8_t const p = me->prio;

    // the AO's prio. must be in range
    Q_REQUIRE_INCRIT(520, (0U < p) && (p <= QF_MAX_ACTIVE));

    // the subscriber AO must be registered (started)
    Q_REQUIRE_INCRIT(540, me == QActive_registry_[p]);

    // the sig parameter must not overlap reserved signals
    Q_REQUIRE_INCRIT(560, sig >= Q_USER_SIG);

    // the unsubscribed signal must be below the maximum of published signals
    Q_REQUIRE_INCRIT(580, (QSignal)sig < QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_UNSUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(me);   // this active object
    QS_END_PRE()

    // remove the AO's prio. from the subscriber set for the signal
    QPSet_remove(&QActive_subscrList_[sig].set, p);

    QF_CRIT_EXIT();
}

//............................................................................
//! @protected @memberof QActive
void QActive_unsubscribeAll(QActive const * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    uint8_t const p = me->prio;

    // the AO's prio. must be in range
    Q_REQUIRE_INCRIT(620, (0U < p) && (p <= QF_MAX_ACTIVE));

    // the subscriber AO must be registered (started)
    Q_REQUIRE_INCRIT(640, me == QActive_registry_[p]);

    QSignal const maxPubSig = QActive_maxPubSignal_;

    // the maximum of published signals must not overlap the reserved signals
    Q_REQUIRE_INCRIT(670, maxPubSig >= (QSignal)Q_USER_SIG);

    QF_CRIT_EXIT();

    // remove this AO's prio. from subscriber lists of all published signals
    for (QSignal sig = Q_USER_SIG; sig < maxPubSig; ++sig) {
        QF_CRIT_ENTRY();

        if (QPSet_hasElement(&QActive_subscrList_[sig].set, p)) {
            // remove the AO's prio. from the subscriber set for the signal
            QPSet_remove(&QActive_subscrList_[sig].set, p);

            QS_BEGIN_PRE(QS_QF_ACTIVE_UNSUBSCRIBE, p)
                QS_TIME_PRE();    // timestamp
                QS_SIG_PRE(sig);  // the signal of this event
                QS_OBJ_PRE(me);   // this active object
            QS_END_PRE()
        }
        QF_CRIT_EXIT();

        QF_CRIT_EXIT_NOP(); // prevent merging critical sections
    }
}
