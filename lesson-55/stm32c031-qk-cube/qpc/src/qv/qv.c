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
#include "qp_pkg.h"       // QP package-scope internal interface
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // QS port
    #include "qs_pkg.h"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QV_H_
    #error Source file included in a project NOT based on the QV kernel
#endif // QV_H_

Q_DEFINE_THIS_MODULE("qv")

//............................................................................
QV_Attr QV_priv_;

//............................................................................
//! @static @public @memberof QV
void QV_schedDisable(uint_fast8_t const ceiling) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    if (ceiling > QV_priv_.schedCeil) { // raising the scheduler ceiling?

        QS_BEGIN_PRE(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE();   // timestamp
            // the previous sched ceiling & new sched ceiling
            QS_2U8_PRE((uint8_t)QV_priv_.schedCeil,
                        (uint8_t)ceiling);
        QS_END_PRE()

        QV_priv_.schedCeil = ceiling;
    }
    QF_CRIT_EXIT();
}

//............................................................................
//! @static @public @memberof QV
void QV_schedEnable(void) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    if (QV_priv_.schedCeil != 0U) { // actually enabling the scheduler?

        QS_BEGIN_PRE(QS_SCHED_UNLOCK, 0U)
            QS_TIME_PRE(); // timestamp
            // current sched ceiling (old), previous sched ceiling (new)
            QS_2U8_PRE((uint8_t)QV_priv_.schedCeil, 0U);
        QS_END_PRE()

        QV_priv_.schedCeil = 0U;
    }
    QF_CRIT_EXIT();
}

//............................................................................
//! @static @public @memberof QF
void QF_init(void) {
    QF_bzero_(&QF_priv_,                 sizeof(QF_priv_));
    QF_bzero_(&QV_priv_,                 sizeof(QV_priv_));
    QF_bzero_(&QActive_registry_[0],     sizeof(QActive_registry_));

    QTimeEvt_init(); // initialize QTimeEvts

#ifdef QV_INIT
    QV_INIT(); // port-specific initialization of the QV kernel
#endif
}

//............................................................................
//! @static @public @memberof QF
void QF_stop(void) {
    QF_onCleanup(); // application-specific cleanup callback
    // nothing else to do for the QV kernel
}

//............................................................................
//! @static @public @memberof QF
int_t QF_run(void) {
    QF_INT_DISABLE();
#ifdef Q_SPY
    // produce the QS_QF_RUN trace record
    QS_beginRec_((uint_fast8_t)QS_QF_RUN);
    QS_endRec_();
#endif // Q_SPY

#ifdef QV_START
    QV_START(); // port-specific startup of the QV kernel
#endif

#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
    uint_fast8_t pprev = 0U; // previously used prio.

#ifdef QF_ON_CONTEXT_SW
    // officially switch to the idle cotext
    QF_onContextSw((QActive *)0, (QActive *)0);
#endif // def QF_ON_CONTEXT_SW

#endif // (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)

    QF_INT_ENABLE();

    QF_onStartup(); // app. callback: configure and enable interrupts

    QF_INT_DISABLE();
    for (;;) { // QV event loop...
        // find the maximum prio. AO ready to run
        uint_fast8_t const p = (QPSet_notEmpty(&QV_priv_.readySet)
                               ? QPSet_findMax(&QV_priv_.readySet)
                               : 0U);

        if (p > QV_priv_.schedCeil) { // is it above the sched ceiling?
            QActive * const a = QActive_registry_[p];

#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
            QS_BEGIN_PRE(QS_SCHED_NEXT, p)
                QS_TIME_PRE();     // timestamp
                QS_2U8_PRE((uint8_t)p,
                            (uint8_t)pprev);
            QS_END_PRE()

#ifdef QF_ON_CONTEXT_SW
            QF_onContextSw(((pprev != 0U)
                            ? QActive_registry_[pprev]
                            : (QActive *)0), a);
#endif // QF_ON_CONTEXT_SW

            pprev = p; // update previous prio.
#endif // (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)

            QF_INT_ENABLE();

            QEvt const * const e = QActive_get_(a);

            // dispatch event (virtual call)
            (*a->super.vptr->dispatch)(&a->super, e, p);
#if (QF_MAX_EPOOL > 0U)
            QF_gc(e);
#endif
            QF_INT_DISABLE();

            if (a->eQueue.frontEvt == (QEvt *)0) { // empty queue?
                QPSet_remove(&QV_priv_.readySet, p);
            }
        }
        else { // no AO ready to run --> idle
#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
            if (pprev != 0U) {
                QS_BEGIN_PRE(QS_SCHED_IDLE, pprev)
                    QS_TIME_PRE();    // timestamp
                    QS_U8_PRE((uint8_t)pprev); // previous prio
                QS_END_PRE()

#ifdef QF_ON_CONTEXT_SW
                QF_onContextSw(QActive_registry_[pprev], (QActive *)0);
#endif // QF_ON_CONTEXT_SW

                pprev = 0U; // update previous prio.
            }
#endif // (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)

            // QV_onIdle() must be called with interrupts DISABLED because
            // the determination of the idle condition can change at any time
            // by an interrupt posting events to a queue.
            //
            // NOTE: QV_onIdle() MUST enable interrupts internally, ideally
            // atomically with putting the CPU into a power-saving mode.
            QV_onIdle();

            QF_INT_DISABLE(); // disable interrupts before looping back
        }
    }
#ifdef __GNUC__ // GNU compiler?
    return 0;
#endif
}

//============================================================================
//! @public @memberof QActive
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    uint_fast16_t const qLen,
    void * const stkSto,
    uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);  // not needed in QV
    Q_UNUSED_PAR(stkSize); // not needed in QV

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300, (me->super.vptr != (struct QAsmVtable *)0)
        && (stkSto == (void *)0));
    QF_CRIT_EXIT();

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-prio. of the AO
    me->pthre = 0U; // not used
    QActive_register_(me); // make QF aware of this active object

    QEQueue_init(&me->eQueue, qSto, qLen); // init the built-in queue

    // top-most initial tran. (virtual call)
    (*me->super.vptr->init)(&me->super, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host
}
