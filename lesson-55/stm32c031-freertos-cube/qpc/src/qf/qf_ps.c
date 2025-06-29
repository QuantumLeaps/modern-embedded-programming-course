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

//............................................................................
//! @static @public @memberof QActive
void QActive_psInit(
    QSubscrList * const subscrSto,
    enum_t const maxSignal)
{
    Q_REQUIRE_INCRIT(100, subscrSto != (QSubscrList *)0);
    Q_REQUIRE_INCRIT(110, maxSignal >= Q_USER_SIG);

    QActive_subscrList_   = subscrSto;
    QActive_maxPubSignal_ = (QSignal)maxSignal;

    // initialize the subscriber list
    for (enum_t sig = 0; sig < maxSignal; ++sig) {
        QPSet_setEmpty(&subscrSto[sig].set);
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

    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

    QSignal const sig = e->sig;
    Q_REQUIRE_INCRIT(240, sig < QActive_maxPubSignal_);

    // make a local, modifiable copy of the subscriber set
    QPSet subscrSet = QActive_subscrList_[sig].set;

    QS_BEGIN_PRE(QS_QF_PUBLISH, qsId)
        QS_TIME_PRE();          // the timestamp
        QS_OBJ_PRE(sender);     // the sender object
        QS_SIG_PRE(sig);        // the signal of the event
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
    QS_END_PRE()

    // is it a mutable event?
    if (e->poolNum_ != 0U) {
        // NOTE: The reference counter of a mutable event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF_gc()) decrements the reference counter and
        // recycles the event if the counter drops to zero. This covers the
        // case when the event was published without any subscribers.
        Q_ASSERT_INCRIT(260, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e);
    }

    QF_CRIT_EXIT();

    if (QPSet_notEmpty(&subscrSet)) { // any subscribers?
        // highest-prio subscriber
        uint8_t p = (uint8_t)QPSet_findMax(&subscrSet);

        QF_CRIT_ENTRY();

        QActive *a = QActive_registry_[p];
        Q_ASSERT_INCRIT(300, a != (QActive *)0);

        QF_CRIT_EXIT();

        QF_SCHED_STAT_
        QF_SCHED_LOCK_(p); // lock the scheduler up to AO's prio

        uint_fast8_t lbound = QF_MAX_ACTIVE + 1U; // fixed loop bound
        for (;;) { // loop over all subscribers

            // QACTIVE_POST() asserts internally if the queue overflows
            QACTIVE_POST(a, e, sender);

            QPSet_remove(&subscrSet, p); // remove the handled subscriber
            if (QPSet_isEmpty(&subscrSet)) {  // no more subscribers?
                break;
            }

            p = (uint8_t)QPSet_findMax(&subscrSet); // highest-prio subscriber

            QF_CRIT_ENTRY();

            a = QActive_registry_[p];
            // the AO must be registered with the framework
            Q_ASSERT_INCRIT(340, a != (QActive *)0);

            --lbound; // fixed loop bound
            Q_INVARIANT_INCRIT(370, lbound > 0U);

            QF_CRIT_EXIT();
        }

        QF_SCHED_UNLOCK_(); // unlock the scheduler
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
#if (QF_MAX_EPOOL > 0U)
    QF_gc(e); // recycle the event to avoid a leak
#endif
}

//............................................................................
//! @protected @memberof QActive
void QActive_subscribe(QActive const * const me,
    enum_t const sig)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // check this AO ("me") and the registration...
    uint8_t const p = me->prio;
    Q_REQUIRE_INCRIT(420, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(440, me == QActive_registry_[p]);

    // check sig parameter and subscription list...
    Q_REQUIRE_INCRIT(460, sig >= Q_USER_SIG);
    Q_REQUIRE_INCRIT(480, (QSignal)sig < QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_SUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(me);   // this active object
    QS_END_PRE()

    // insert the prio. into the subscriber set
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

    // check this AO ("me") and the registration...
    uint8_t const p = me->prio;
    Q_REQUIRE_INCRIT(520, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(540, me == QActive_registry_[p]);

    // check sig parameter and subscription list...
    Q_REQUIRE_INCRIT(560, sig >= Q_USER_SIG);
    Q_REQUIRE_INCRIT(580, (QSignal)sig < QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_UNSUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(me);   // this active object
    QS_END_PRE()

    // remove the prio. from the subscriber set
    QPSet_remove(&QActive_subscrList_[sig].set, p);

    QF_CRIT_EXIT();
}

//............................................................................
//! @protected @memberof QActive
void QActive_unsubscribeAll(QActive const * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // check this AO ("me") and the registration...
    uint_fast8_t const p = (uint_fast8_t)me->prio;
    Q_REQUIRE_INCRIT(620, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(640, me == QActive_registry_[p]);

    // check maxPubSignal_ and subscription list...
    QSignal const maxPubSig = QActive_maxPubSignal_;
    Q_REQUIRE_INCRIT(670, maxPubSig >= (QSignal)Q_USER_SIG);

    QF_CRIT_EXIT();

    for (QSignal sig = Q_USER_SIG; sig < maxPubSig; ++sig) {
        QF_CRIT_ENTRY();

        if (QPSet_hasElement(&QActive_subscrList_[sig].set, p)) {
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
