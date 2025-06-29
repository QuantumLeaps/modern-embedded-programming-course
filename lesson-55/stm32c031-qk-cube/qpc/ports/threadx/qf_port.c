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
    #include "qs_pkg.h"   // QS package-scope internal interface
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

Q_DEFINE_THIS_MODULE("qf_port")

//............................................................................
void QF_init(void) {
    QF_bzero_(&QF_priv_,                 sizeof(QF_priv_));
    QF_bzero_(&QActive_registry_[0],     sizeof(QActive_registry_));
    QTimeEvt_init(); // initialize QTimeEvts
}
//............................................................................
int_t QF_run(void) {
    QF_onStartup();  // QF callback to configure and start interrupts

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();

    return 0; // return success
}
//............................................................................
void QF_stop(void) {
    QF_onCleanup(); // cleanup callback
}
//............................................................................
static void thread_function(ULONG thread_input) { // ThreadX signature
    QActive *act = (QActive *)thread_input;

    // event-loop
    for (;;) { // for-ever
        QEvt const *e = QActive_get_(act);
        QASM_DISPATCH(&act->super, e, act->prio);
        QF_gc(e); // check if the event is garbage, and collect it if so
    }
}
//............................................................................
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, uint_fast16_t const qLen,
    void * const stkSto, uint_fast16_t const stkSize,
    void const * const par)
{
    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority
    me->pthre = (uint8_t)(prioSpec >> 8U); // QF preemption-threshold
    QActive_register_(me); // make QF aware of this AO

    // create the ThreadX message queue for the AO
    UINT tx_err = tx_queue_create(&me->eQueue,
        me->thread.tx_thread_name,
        TX_1_ULONG,
        (VOID *)qSto,
        (ULONG)(qLen * sizeof(QEvtPtr)));

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(110, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();

    // top-most initial tran. (virtual call)
    QASM_INIT(&me->super, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host

    UINT tx_prio = QF_TO_TX_PRIO_MAP(me->prio);
    UINT tx_pt   = QF_TO_TX_PRIO_MAP(me->pthre);
    tx_err = tx_thread_create(
        &me->thread, // ThreadX thread control block
        me->thread.tx_thread_name, // unique thread name
        &thread_function,        // thread function
        (ULONG)me,               // thread parameter
        stkSto,                  // stack start
        stkSize,                 // stack size in bytes
        tx_prio,                 // ThreadX priority
        tx_pt,                   // ThreadX preempt-threshold, see NOTE1
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(120, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();
}
//............................................................................
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    // this function must be called before QActive_start(),
    // which implies that me->thread.tx_thread_name must not be used yet;
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(150, me->thread.tx_thread_name == (char *)0);

    switch (attr1) {
        case THREAD_NAME_ATTR:
            // temporarily store the name, cast 'const' away
            me->thread.tx_thread_name = (char *)attr2;
            break;
        // ...
        default:
            break;
    }
    QF_CRIT_EXIT();
}
//............................................................................
bool QActive_post_(QActive * const me, QEvt const * const e,
                   uint_fast16_t const margin, void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

    uint_fast16_t nFree = (uint_fast16_t)me->eQueue.tx_queue_available_storage;

    bool status;
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_INCRIT(210); // must be able to post the event
        }
    }
    else if (nFree > (QEQueueCtr)margin) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, me->prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(me);      // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()

        if (e->poolNum_ != 0U) { // is it a pool event?
            Q_ASSERT_INCRIT(205, e->refCtr_ < (2U * QF_MAX_ACTIVE));
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        QF_CRIT_EXIT();

        UINT tx_err = tx_queue_send(&me->eQueue, (VOID *)&e, TX_NO_WAIT);

        QF_CRIT_ENTRY();
        // posting to the ThreadX message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(220, tx_err == TX_SUCCESS);
        QF_CRIT_EXIT();
    }
    else {

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, me->prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(me);      // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()

        QF_CRIT_EXIT();
    }

    return status;
}
//............................................................................
void QActive_postLIFO_(QActive * const me, QEvt const * const e) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(300, e != (QEvt *)0);

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(me->eQueue.tx_queue_available_storage); // # free
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    if (e->poolNum_ != 0U) { // is it a pool event?
        Q_ASSERT_INCRIT(305, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
    QF_CRIT_EXIT();

    UINT tx_err = tx_queue_front_send(&me->eQueue, (VOID *)&e, TX_NO_WAIT);

    QF_CRIT_ENTRY();
    // LIFO posting must succeed, see NOTE3
    Q_ASSERT_INCRIT(310, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();
}
//............................................................................
QEvt const *QActive_get_(QActive * const me) {
    QEvtPtr e;
    UINT tx_err = tx_queue_receive(&me->eQueue, (VOID *)&e, TX_WAIT_FOREVER);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(710, tx_err == TX_SUCCESS);

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, me->prio)
        QS_TIME_PRE();          // timestamp
        QS_SIG_PRE(e->sig);     // the signal of this event
        QS_OBJ_PRE(me);         // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(me->eQueue.tx_queue_available_storage);// # free
    QS_END_PRE()
    QF_CRIT_EXIT();

    return e;
}

//............................................................................
void QFSchedLock_(QFSchedLock * const lockStat, uint_fast8_t prio) {

    lockStat->lockHolder = tx_thread_identify();

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // must be in thread context, so current TX thread must be valid
    Q_REQUIRE_INCRIT(800, lockStat->lockHolder != (TX_THREAD *)0);
    QF_CRIT_EXIT();

    // change the preemption threshold of the current thread
    UINT tx_err = tx_thread_preemption_change(lockStat->lockHolder,
                     QF_TO_TX_PRIO_MAP(prio),
                     &lockStat->prevThre);

    if (tx_err == TX_SUCCESS) {
        lockStat->lockPrio = prio;

        QF_CRIT_ENTRY();
        QS_BEGIN_PRE(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE(); // timestamp
            QS_2U8_PRE(TX_TO_QF_PRIO_MAP(lockStat->prevThre),
                        prio); // new lock prio
        QS_END_PRE()
        QF_CRIT_EXIT();
    }
    else if (tx_err == TX_THRESH_ERROR) {
        // threshold was greater than (lower prio) than the current prio
        lockStat->lockPrio = 0U; // threshold not changed
    }
    else {
        // no other errors are tolerated
        QF_CRIT_ENTRY();
        Q_ERROR_INCRIT(810);
        //QF_CRIT_EXIT();
    }
}

//............................................................................
void QFSchedUnlock_(QFSchedLock const * const lockStat) {

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // the lock holder must be valid and the scheduler must be locked
    Q_REQUIRE_INCRIT(900, (lockStat->lockHolder != (TX_THREAD *)0)
                          && (lockStat->lockPrio != 0U));

    QS_BEGIN_PRE(QS_SCHED_UNLOCK, 0U)
        QS_TIME_PRE(); // timestamp
        QS_2U8_PRE(lockStat->lockPrio, // prev lock prio
                    TX_TO_QF_PRIO_MAP(lockStat->prevThre)); // new lock prio
    QS_END_PRE()
    QF_CRIT_EXIT();

    // restore the preemption threshold of the lock holder
    UINT old_thre;
    UINT tx_err = tx_thread_preemption_change(lockStat->lockHolder,
                     lockStat->prevThre,
                     &old_thre);
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(910, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();
}

//============================================================================
// NOTE1:
// When QF preemption threshold is NOT specified (upper byte of prio is 0),
// QActive_register_() will set me->pthre equal to me->prio, which means
// that tx_pt == tx_prio. In that case the call to ThreadX tx_thread_create()
// will disable the ThreadX preemption-threshold for this thread.
//
// NOTE3:
// The event posting to ThreadX message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
