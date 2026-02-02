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

Q_DEFINE_THIS_MODULE("qf_defer")

//............................................................................
//! @protected @memberof QActive
bool QActive_defer(QActive const * const me,
    struct QEQueue * const eq,
    QEvt const * const e)
{
    // post with margin==0U to use all available entries in the queue
    bool const status = QEQueue_post(eq, e, 0U, me->prio);

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    if (status) { // deferring successful?
        QS_BEGIN_PRE(QS_QF_ACTIVE_DEFER, me->prio)
            QS_TIME_PRE();      // time stamp
            QS_OBJ_PRE(me);     // this active object
            QS_OBJ_PRE(eq);     // the deferred queue
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_END_PRE()
    }
    else { // deferring failed
        QS_BEGIN_PRE(QS_QF_ACTIVE_DEFER_ATTEMPT, me->prio)
            QS_TIME_PRE();      // time stamp
            QS_OBJ_PRE(me);     // this active object
            QS_OBJ_PRE(eq);     // the deferred queue
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_END_PRE()
    }
    QS_CRIT_EXIT();

    return status;
}

//............................................................................
//! @protected @memberof QActive
bool QActive_recall(QActive * const me,
    struct QEQueue * const eq)
{
    bool recalled = false; // assume failure

    QEvt const * const e = QEQueue_get(eq, me->prio);
    if (e != (QEvt *)0) { // event available?

        // post it to the front of the AO's queue.
        // NOTE: asserts internally if the posting fails.
        QACTIVE_POST_LIFO(me, e);

        QF_CRIT_STAT
        QF_CRIT_ENTRY();

        if (e->poolNum_ != 0U) { // mutable event?

            // after posting to the AO's queue, the event must be referenced
            // at least twice: once in the deferred event queue (eq->get()
            // did NOT decrement the reference counter) and once in the
            // AO's event queue.
            Q_ASSERT_INCRIT(210, e->refCtr_ >= 2U);

            // decrement the reference counter once, to account for removing
            // the event from the deferred queue.
            QEvt_refCtr_dec_(e); // decrement the reference counter
        }

        QS_BEGIN_PRE(QS_QF_ACTIVE_RECALL, me->prio)
            QS_TIME_PRE();      // time stamp
            QS_OBJ_PRE(me);     // this active object
            QS_OBJ_PRE(eq);     // the deferred queue
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_END_PRE()

        QF_CRIT_EXIT();

        recalled = true; // success
    }
    else {
        QS_CRIT_STAT
        QS_CRIT_ENTRY();

        QS_BEGIN_PRE(QS_QF_ACTIVE_RECALL_ATTEMPT, me->prio)
            QS_TIME_PRE();      // time stamp
            QS_OBJ_PRE(me);     // this active object
            QS_OBJ_PRE(eq);     // the deferred queue
        QS_END_PRE()

        QS_CRIT_EXIT();
    }
    return recalled;
}

//............................................................................
//! @protected @memberof QActive
uint16_t QActive_flushDeferred(QActive const * const me,
    struct QEQueue * const eq,
    uint_fast16_t const num)
{
    uint16_t n = 0U; // the flushed event counter
    while (n < num) { // below the requested number?
        QEvt const * const e = QEQueue_get(eq, me->prio);
        if (e != (QEvt *)0) { // event obtained from the queue?
            ++n; // count one more flushed event
#if (QF_MAX_EPOOL > 0U)
            QF_gc(e); // garbage collect
#endif
        }
        else { // queue ran out of events
            break; // done flushing
        }
    }

    return n;
}
