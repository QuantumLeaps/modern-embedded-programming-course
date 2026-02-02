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
#define QP_IMPL             // this is QP implementation
#include "qp_port.h"        // QP port
#include "qp_pkg.h"         // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.h"    // QS port
    #include "qs_pkg.h"     // QS package-scope internal interface
#else
    #include "qs_dummy.h"   // disable the QS software tracing
#endif // Q_SPY

Q_DEFINE_THIS_MODULE("qf_port")

//----------------------------------------------------------------------------
static void thread_main(void *p1, void *p2, void *p3);  // prototype
static void thread_main(void *p1, void *p2, void *p3) { // Zephyr thread
    Q_UNUSED_PAR(p2);
    Q_UNUSED_PAR(p3);
    QActive * const act = (QActive *)p1;

    // the event-loop...
    for (;;) { // for-ever
        QEvt const * const e = QActive_get_(act); // WAIT for event
        QASM_DISPATCH(act, e, act->prio); // dispatch event (virtual call)
#if (QF_MAX_EPOOL > 0U)
        QF_gc(e); // check if the event is garbage, and collect it if so
#endif
    }
#ifdef QACTIVE_CAN_STOP
    //QActive_unregister_(act); // remove this object from the framewrok
#endif
}

//============================================================================
// Active Object customization...

//............................................................................
//! @private @memberof QActive
bool QActive_post_(QActive * const me,
    QEvt const * const e,
    uint_fast16_t const margin,
    void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the event to post must not be NULL
    Q_REQUIRE_INCRIT(100, e != (QEvt *)0);

    // the number of free slots available in the Zephyr queue
    // NOTE: k_msgq_num_free_get() can be safely called from crit.sect.
    uint_fast16_t const nFree =
        (uint_fast16_t)k_msgq_num_free_get(&me->eQueue);

    bool status = ((margin == QF_NO_MARGIN) || (nFree > (QEQueueCtr)margin));

    if (status) { // should try to post the event?
#if (QF_MAX_EPOOL > 0U)
        if (e->poolNum_ != 0U) { // is it a mutable event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
#endif // (QF_MAX_EPOOL > 0U)

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, me->prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(me);     // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(0U);     // min # free entries (unknown)
        QS_END_PRE()

        QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

        // post the event to the Zephyr event queue, see NOTE3
        status = (k_msgq_put(&me->eQueue, (void const *)&e, K_NO_WAIT)
                  == 0);
    }

    if (!status) { // event NOT posted?
        QF_CRIT_ENTRY();

        // posting is allowed to fail only when margin != QF_NO_MARGIN
        Q_ASSERT_INCRIT(130, margin != QF_NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, me->prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(me);     // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(margin); // margin requested
        QS_END_PRE()

        QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
        QF_gc(e); // recycle the event to avoid a leak
#endif
    }

    return status;
}
//............................................................................
//! @private @memberof QActive
void QActive_postLIFO_(QActive * const me, QEvt const * const e) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the posted event must be be valid (which includes not NULL)
    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Id & ref-Count
        QS_EQC_PRE(k_msgq_num_free_get(&me->eQueue)); // # free entries
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

    int const err = k_msgq_put_front(&me->eQueue, (void const *)&e);

#ifndef Q_UNSAFE
    QF_CRIT_ENTRY();
    // LIFO posting to the Zephyr queue must succeed, see NOTE3
    Q_ASSERT_INCRIT(230, err == 0);
    QF_CRIT_EXIT();
#else
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
//! @private @memberof QActive
QEvt const *QActive_get_(QActive * const me) {
    // wait for an event (forever)
    QEvtPtr e;
    int const err = k_msgq_get(&me->eQueue, (void *)&e, K_FOREVER);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    Q_ASSERT_INCRIT(310, err == 0); // queue-get must succeed
#else
    Q_UNUSED_PAR(err);
#endif

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Id & ref-Count
        QS_EQC_PRE(k_msgq_num_free_get(&me->eQueue)); // # free entries
    QS_END_PRE()

    QF_CRIT_EXIT();

    return e;
}
//............................................................................
//! @static @public @memberof QActive
uint16_t QActive_getQueueUse(uint_fast8_t const prio) {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
//! @static @public @memberof QActive
uint16_t QActive_getQueueFree(uint_fast8_t const prio) {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
//! @static @public @memberof QActive
uint16_t QActive_getQueueMin(uint_fast8_t const prio) {
    Q_UNUSED_PAR(prio);
    return 0U; // minimum free entries in a queue not supported in this RTOS
}

//............................................................................
//! @public @memberof QActive
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, uint_fast16_t const qLen,
    void * const stkSto, uint_fast16_t const stkSize,
    void const * const par)
{
    // extract data temporarily saved in QActive_setAttr()
    uint32_t const opt = me->eQueue.used_msgs;
#ifdef CONFIG_THREAD_NAME
    char const * const name = (char const *)me->thread.init_data;
#endif

    // initialize the Zephyr message queue
    k_msgq_init(&me->eQueue, (char *)qSto, sizeof(QEvtPtr), (uint32_t)qLen);

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = 0U; // preemption-threshold (not used for AO registration)
    QActive_register_(me); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    QASM_INIT(me, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // Zehyr priority, see NOTE1
    int zephyr_prio = (int)(prioSpec >> 8U);
    if (zephyr_prio == 0) {
        zephyr_prio = (int)(QF_MAX_ACTIVE - me->prio);
    }

    // create a Zephyr thread for the AO...
    k_thread_create(&me->thread,
                    (k_thread_stack_t *)stkSto,
                    (size_t)stkSize,
                    &thread_main,
                    (void *)me, // p1
                    (void *)0,  // p2
                    (void *)0,  // p3
                    zephyr_prio,// Zephyr priority
                    opt,        // thread options
                    K_NO_WAIT); // start immediately, see NOTE4

#ifdef CONFIG_THREAD_NAME
    // set the Zephyr thread name, if initialized, or the default name "AO"
    k_thread_name_set(&me->thread, (name != (char *)0) ? name : "AO");
#endif
}
//............................................................................
//! @public @memberof QActive
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    // see NOTE2
    me->eQueue.used_msgs = attr1; // will be used for thread options
#ifdef CONFIG_THREAD_NAME
    me->thread.init_data = (void *)attr2; // will be used for thread name
#else
    Q_UNUSED_PAR(attr2);
#endif
}

//============================================================================
// Zephyr spinlock for QF critical section
struct k_spinlock QF_spinlock = (struct k_spinlock){};

//............................................................................
void QF_init(void) {
    k_sched_lock(); // lock the Zephyr scheduler until QF_run(), see NOTE4

    QTimeEvt_init(); // initialize QTimeEvts
}
//............................................................................
int_t QF_run(void) {
    QF_onStartup(); // the startup callback

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();
#endif // Q_SPY

    // unlock the Zephyr scheduler to start the AOs/threads, see NOTE4
    k_sched_unlock();

#if defined(QF_IDLE) || defined(Q_SPY)

#if (CONFIG_NUM_PREEMPT_PRIORITIES > 0)
    // lower the priority of the main thread to the level of idle thread
    k_thread_priority_set(k_current_get(),
        CONFIG_NUM_PREEMPT_PRIORITIES - 1);
#endif

    for (;;) { // idle thread
        QF_onIdle(); // idle processing
    }

#else // QF idle thread not configured
    return 0; // return from the main Zephyr thread
#endif
}
//............................................................................
void QF_stop(void) {
    QF_onCleanup(); // cleanup callback
}

//============================================================================
// NOTE1:
// The Zephyr priority of the AO thread can be specified in two ways:
//
// A. Implicitly based on the AO's priority (Zephyr uses the reverse
//    priority numbering scheme than QP). This option is chosen, when
//    the higher-byte of the prioSpec parameter is set to zero.
//
// B. Explicitly as the higher-byte of the prioSpec parameter.
//    This option is chosen when the prioSpec parameter is not-zero.
//    For example, Q_PRIO(10U, -1U) will explicitly specify AO priority
//    as 10 and Zephyr priority as -1.
//
// CAUTION: The explicit Zephyr priority is NOT sanity-checked, so it is the
// responsibility of the application to ensure that it is consistent with the
// QP priority. An example of inconsistent setting would be assigning Zephyr
// priorities that would result in a different relative prioritization of AOs
// than indicated by the QP priorities assigned to the AOs.
//
// NOTE2:
// In the Zephyr port, the generic function QActive_setAttr() is used to set
// the options for the Zephyr thread as follows (see also QActive_start()):
// - attr1 - will be used for thread options in k_thread_create()
// - attr2 - will be used for thread name in k_thread_name_set()
// CAUTION: QActive_setAttr() needs to be called *before* QActive_start() for
// the given AO.
//
// NOTE3:
// The event posting to Zephyr message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
// NOTE4:
// Durring the initial transient (from QF_init() till QF_run()), the Zephyr
// scheduler remains *locked*, to ensure that the threads created during that
// time start running only after the control is passed to the framework (after
// calling QF_run()).
//
