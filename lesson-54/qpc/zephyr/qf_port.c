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
struct k_spinlock QF_spinlock;

//............................................................................
void QF_init(void) {
    QF_spinlock = (struct k_spinlock){};

    QF_bzero_(&QF_priv_,             sizeof(QF_priv_));
    QF_bzero_(&QActive_registry_[0], sizeof(QActive_registry_));
    QTimeEvt_init(); // initialize QTimeEvts
}
//............................................................................
int_t QF_run(void) {
    QF_onStartup();

#ifdef Q_SPY

#if (CONFIG_NUM_PREEMPT_PRIORITIES > 0)
    // lower the priority of the main thread to the level of idle thread
    k_thread_priority_set(k_current_get(),
                          CONFIG_NUM_PREEMPT_PRIORITIES - 1);
#endif

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();

    // perform QS work...
    while (true) {
        QS_rxParse();   // parse any QS-RX bytes
        QS_doOutput();  // perform the QS-TX output
    }
#else
    return 0; // return from the main Zephyr thread
#endif
}
//............................................................................
void QF_stop(void) {
    QF_onCleanup();  // cleanup callback
}

//............................................................................
static void thread_entry(void *p1, void *p2, void *p3) {
    Q_UNUSED_PAR(p2);
    Q_UNUSED_PAR(p3);
    QActive *act = (QActive *)p1;

    // event-loop
    for (;;) { // for-ever
        QEvt const *e = QActive_get_(act);
        // dispatch event (virtual call)
        (*act->super.vptr->dispatch)(&act->super, e, act->prio);
        QF_gc(e); // check if the event is garbage, and collect it if so
    }
}

//............................................................................
//
// In the Zephyr port the generic function QActive_setAttr() is used to
// set the options for the Zephyr thread (attr1) and thread name (attr2).
// QActive_setAttr() needs to be called *before* QActive_start() for the
// given active object.
//
// In this Zephyr port the attributes will be used as follows
// (see also QActive_start()):
// - attr1 - will be used for thread options in k_thread_create()
// - attr2 - will be used for thread name in k_thread_name_set()
//
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    me->thread.base.order_key = attr1; // will be used for thread options
    me->thread.init_data = (void *)attr2; // will be used for thread name
}
//............................................................................
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, uint_fast16_t const qLen,
    void * const stkSto, uint_fast16_t const stkSize,
    void const * const par)
{
    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = 0U;   // preemption-threshold (not used for AO registration)
    QActive_register_(me); // make QF aware of this active object

    // initialize the Zephyr message queue
    k_msgq_init(&me->eQueue, (char *)qSto, sizeof(QEvtPtr), (uint32_t)qLen);

    // top-most initial tran. (virtual call)
    (*me->super.vptr->init)(&me->super, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // The Zephyr priority of the AO thread can be specified in two ways:
    //
    // 1. Implictily based on the AO's priority (Zephyr uses the reverse
    //    priority numbering scheme than QP). This option is chosen, when
    //    the higher-byte of the prioSpec parameter is set to zero.
    //
    // 2. Explicitly as the higher-byte of the prioSpec parameter.
    //    This option is chosen when the prioSpec parameter is not-zero.
    //    For example, Q_PRIO(10U, -1U) will explicitly specify AO priority
    //    as 10 and Zephyr priority as -1.
    //
    //    NOTE: The explicit Zephyr priority is NOT sanity-checked,
    //    so it is the responsibility of the application to ensure that
    //    it is consistent with the AO's priority. An example of
    //    inconsistent setting would be assigning Zephyr priorities that
    //    would result in a different relative priritization of AO's threads
    //    than indicated by the AO priorities assigned.
    //
    int zephyr_prio = (int)((int16_t)prioSpec >> 8);
    if (zephyr_prio == 0) {
        zephyr_prio = (int)QF_MAX_ACTIVE - (int)me->prio;
    }

    // extract data temporarily saved in me->thread by QActive_setAttr()
    uint32_t opt = me->thread.base.order_key;
#ifdef CONFIG_THREAD_NAME
    char const *name = (char const *)me->thread.init_data;
#endif

    // clear the Zephyr thread structure before creating the thread
    me->thread = (struct k_thread){};

    // create a Zephyr thread for the AO...
    k_thread_create(&me->thread,
                    (k_thread_stack_t *)stkSto,
                    (size_t)stkSize,
                    &thread_entry,
                    (void *)me, // p1
                    (void *)0,  // p2
                    (void *)0,  // p3
                    zephyr_prio,// Zephyr priority
                    opt,        // thread options
                    K_NO_WAIT); // start immediately

#ifdef CONFIG_THREAD_NAME
    // set the Zephyr thread name, if initialized, or the default name "AO"
    k_thread_name_set(&me->thread, (name != (char *)0) ? name : "AO");
#endif
}
//............................................................................
bool QActive_post_(QActive * const me, QEvt const * const e,
                   uint_fast16_t const margin, void const * const sender)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

    // NOTE: k_msgq_num_free_get() can be safely called from crit-section
    uint_fast16_t nFree = (uint_fast16_t)k_msgq_num_free_get(&me->eQueue);

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
            QS_2U8_PRE(e->poolNum_, e->refCtr_);// pool-Id & ref-Count
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()

        if (e->poolNum_ != 0U) { // is it a pool event?
            Q_ASSERT_INCRIT(205, e->refCtr_ < (2U * QF_MAX_ACTIVE));
            QEvt_refCtr_inc_(e); // increment the reference counter
        }

        QF_CRIT_EXIT(); // exit crit.sect. before calling Zephyr API
        int err = k_msgq_put(&me->eQueue, (void const *)&e, K_NO_WAIT);
        QF_CRIT_ENTRY(); // re-enter crit.sect. after calling Zephyr API

        // posting to the Zephyr message queue must succeed, see NOTE1
        Q_ASSERT_INCRIT(220, err == 0);
#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
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
    }
    QF_CRIT_EXIT();

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
        QS_EQC_PRE(k_msgq_num_free_get(&me->eQueue)); // # free entries
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    if (e->poolNum_ != 0U) { // is it a pool event?
        Q_ASSERT_INCRIT(305, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    // NOTE: Zephyr message queue does not currently support LIFO posting
    // so normal FIFO posting is used instead.
    QF_CRIT_EXIT(); // exit crit.sect. before calling Zephyr API
    int err = k_msgq_put(&me->eQueue, (void *)&e, K_NO_WAIT);
    QF_CRIT_ENTRY(); // re-enter crit.sect. after calling Zephyr API

    Q_ASSERT_INCRIT(310, err == 0);
#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif

    QF_CRIT_EXIT();
}
//............................................................................
QEvt const *QActive_get_(QActive * const me) {
    // wait for an event (forever)
    QEvtPtr e;
    int err = k_msgq_get(&me->eQueue, (void *)&e, K_FOREVER);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_ASSERT_INCRIT(410, err == 0); // queue-get must succeed
#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(k_msgq_num_free_get(&me->eQueue));// # free entries
    QS_END_PRE()

    QF_CRIT_EXIT();

    return e;
}
