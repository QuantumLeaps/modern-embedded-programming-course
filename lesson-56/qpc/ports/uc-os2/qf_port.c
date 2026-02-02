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
static void task_main(void *pdata);  // prototype
static void task_main(void *pdata) { // uC-OS2 task signature
    QActive * const act = (QActive *)pdata;

    // event-loop...
    for (;;) { // for-ever
        QEvt const * const e = QActive_get_(act); // WAIT for event
        QASM_DISPATCH(act, e, act->prio); // dispatch event (virtual call)
        QF_gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    //QActive_unregister_(act); // remove this object from the framewrok
#endif
}

//============================================================================
// Active Object customization...

//............................................................................
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

    // the number of free slots available in the uC-OS2 queue
    uint_fast16_t const nFree =
        (uint_fast16_t)(((OS_Q_DATA *)me->eQueue)->OSQSize
         - ((OS_Q_DATA *)me->eQueue)->OSNMsgs);

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

        // post the event to the uC-OS2 event queue, see NOTE3
        status = (OSQPost(me->eQueue, (void *)e) == OS_ERR_NONE);
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
        QS_EQC_PRE(((OS_Q *)me->eQueue)->OSQSize
                     - ((OS_Q *)me->eQueue)->OSQEntries); // # free entries
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

    INT8U const err = OSQPostFront((OS_EVENT *)me->eQueue, (void *)e);

#ifndef Q_UNSAFE
    QF_CRIT_ENTRY();
    // LIFO posting to uC-OS2 message queue must succeed, see NOTE3
    Q_ASSERT_INCRIT(230, err == OS_ERR_NONE);
    QF_CRIT_EXIT();
#else
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
QEvt const *QActive_get_(QActive * const me) {
    // wait for an event (forever)
    INT8U err;
    QEvt const * const e =
        (QEvt const *)OSQPend((OS_EVENT *)me->eQueue, 0U, &err);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    Q_ASSERT_INCRIT(310, err == OS_ERR_NONE); // queue-get must succeed
#else
    Q_UNUSED_PAR(err);
#endif

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Id & ref-Count
        QS_EQC_PRE(((OS_Q *)me->eQueue)->OSQSize
                    - ((OS_Q *)me->eQueue)->OSQEntries); // # free entries
    QS_END_PRE()

    QF_CRIT_EXIT();

    return e;
}
//............................................................................
uint16_t QActive_getQueueUse(uint_fast8_t const prio) {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
uint16_t QActive_getQueueFree(uint_fast8_t const prio) {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
uint16_t QActive_getQueueMin(uint_fast8_t const prio) {
    Q_UNUSED_PAR(prio);
    return 0U; // minimum free entries in a queue not supported in this RTOS
}

//............................................................................
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, uint_fast16_t const qLen,
    void * const stkSto, uint_fast16_t const stkSize,
    void const * const par)
{
    // extract data temporarily saved in QActive_setAttr()
    void * const task_name = (void *)me->eQueue;

    // create the uC-OS2 message queue
    me->eQueue = OSQCreate((void **)qSto, qLen);  // create uC-OS2 queue

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // the uC-OS2 queue must be created correctly
    Q_ASSERT_INCRIT(410, me->eQueue != (OS_EVENT *)0);
    QF_CRIT_EXIT();

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = 0U; // preemption-threshold (not used for AO registration)
    QActive_register_(me); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    QASM_INIT(me, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // uC-OS2 priority, see NOTE1
    INT8U ucos2_prio = (INT8U)(prioSpec >> 8U);
    if (ucos2_prio == 0) {
        ucos2_prio = (INT8U)(OS_LOWEST_PRIO - me->prio);
    }

    // create the uC-OS2 task for the AO...
    INT8U const err = OSTaskCreateExt(&task_main, // the task function
        (void *)me,          // the 'pdata' parameter
#if OS_STK_GROWTH
        &((OS_STK *)stkSto)[(stkSize/sizeof(OS_STK)) - 1], // ptos
#else
        (OS_STK *)stkSto,    // ptos
#endif
        ucos2_prio,          // uC-OS2 task priority
        (INT16U)me->prio,    // the unique AO priority as task ID
#if (OS_STK_GROWTH == 0)
        (OS_STK *)stkSto,    // pbos
#else
        &((OS_STK *)stkSto)[(stkSize/sizeof(OS_STK)) - 1], // pbos
#endif
        (INT32U)(stkSize/sizeof(OS_STK)), // stack size in OS_STK units
        task_name,           // pext
        (INT16U)me->thread); // task options, see NOTE1

    QF_CRIT_ENTRY();
    // uC-OS2 task must be created correctly
    Q_ASSERT_INCRIT(490, err == OS_ERR_NONE);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    // NOTE: this function must be called *before* QActive_start(),
    // which implies that me->thread.tx_thread_name must not be used yet;
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    switch (attr1) {
        case TASK_NAME_ATTR:
            // me->eQueue must not be used yet
            Q_ASSERT_INCRIT(510, me->eQueue == (OS_EVENT *)0);
            // temporarily store the name, cast 'const' away
            me->eQueue = (OS_EVENT *)attr2;
            break;
        // ...
        default:
            me->thread = attr1;
            break;
    }
    QF_CRIT_EXIT();
}

//============================================================================

//............................................................................
void QF_init(void) {
    QTimeEvt_init(); // initialize QTimeEvts
    OSInit();        // initialize uC-OS2
}
//............................................................................
int_t QF_run(void) {
    QF_onStartup(); // the startup callback, see NOTE4

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();
#endif // Q_SPY

    OSStart(); // start uC-OS2 multitasking--should never return

    return 0; // this unreachable return keeps the compiler happy
}
//............................................................................
void QF_stop(void) {
    QF_onCleanup(); // cleanup callback
}

//============================================================================
// NOTE1:
// The uC-OS2 priority of the AO thread can be specified in two ways:
//
// A. Implicitly based on the AO's priority (uC-OS2 uses the reverse
//    priority numbering scheme than QP). This option is chosen when
//    the higher-byte of the prioSpec parameter is set to zero.
//
// B. Explicitly as the higher-byte of the prioSpec parameter.
//    This option is chosen when the prioSpec parameter is not-zero.
//    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
//    as 10 and uC-OS2 priority as 5.
//
// CAUTION: The explicit uC-OS2 priority is NOT sanity-checked, so it is the
// responsibility of the application to ensure that it is consistent with the
// QP priority. An example of inconsistent setting would be assigning uC-OS2
// priorities that would result in a different relative prioritization of AOs
// than indicated by the QP priorities assigned to the AOs.
//
// NOTE2:
// In the uC-OS2 port, the generic function QActive_setAttr() is used to
// set the options for the uC-OS2 task options and task name.
// CAUTION: QActive_setAttr() needs to be called *before* QActive_start() for
// the given AO.
//
// NOTE3:
// The event posting to uC-OS2 message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
// NOTE4:
// The QF_onStartup() should enter the critical section before configuring
// and starting interrupts and it should NOT exit the critical section.
// Thus the interrupts cannot fire until uC-OS2 starts multitasking
// in OSStart(). This is to prevent a (narrow) time window in which interrupts
// could make some tasks ready to run, but the OS would not be ready yet
// to perform context switch.
//
