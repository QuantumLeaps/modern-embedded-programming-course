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

#if ( configSUPPORT_STATIC_ALLOCATION == 0 )
    #error This QP/C port to FreeRTOS requires configSUPPORT_STATIC_ALLOCATION
#endif

#if ( configMAX_PRIORITIES < QF_MAX_ACTIVE )
    #error FreeRTOS configMAX_PRIORITIES must not be less than QF_MAX_ACTIVE
#endif

// Local objects -------------------------------------------------------------
static void task_function(void *pvParameters); // FreeRTOS task signature

// The following macro provides the number of free slots in the FreeRTOS
// queue.
//
// NOTE1:
// The official FreeRTOS API uxQueueSpacesAvailable() is not used
// here, because that API uses task-level critical section internally.
// Instead, the free slots calculation happens here in already
// established critical section. Unfortunately, the bizarre "information
// obfuscating" policy of FreeRTOS (incorrectly called "information
// hiding") forces the use of the StaticQueue_t with "dummy" members.
// This could potentially break in the future releases of FreeRTOS.
//
// Currently, the correspondence between xQUEUE and StaticQueue_t
// is as follows (see queue.c and FreeRTOS.h, respectively):
//
// xQUEUE.uxMessagesWaiting == StaticQueue_t.uxDummy4[0];
// xQUEUE.uxLength          == StaticQueue_t.uxDummy4[1];
//
#define FREERTOS_QUEUE_GET_FREE(me_) \
    ((me_)->osObject.uxDummy4[1] - (me_)->osObject.uxDummy4[0])

//============================================================================
void QF_init(void) {
    QF_bzero_(&QF_priv_,             sizeof(QF_priv_));
    QF_bzero_(&QActive_registry_[0], sizeof(QActive_registry_));
    QTimeEvt_init(); // initialize QTimeEvts
}
//............................................................................
int_t QF_run(void) {
    QF_onStartup(); // the startup callback (configure/enable interrupts)

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QF_CRIT_EXIT();
#endif

    vTaskStartScheduler(); // start the FreeRTOS scheduler

    QF_CRIT_ENTRY();
    Q_ERROR_INCRIT(110); // the FreeRTOS scheduler should never return
    QF_CRIT_EXIT();

    return 0; // dummy return to make the compiler happy
}
//............................................................................
void QF_stop(void) {
    QF_onCleanup(); // cleanup callback
}

//............................................................................
static void task_function(void *pvParameters) { // FreeRTOS task signature
    QActive *act = (QActive *)pvParameters;

#ifdef QACTIVE_CAN_STOP
    while (act->eQueue != (QueueHandle_t)0)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = QActive_get_(act); // wait for event
        QASM_DISPATCH(&act->super, e, act->prio); // dispatch to the SM
        QF_gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    QActive_unregister_(act); // remove this object from the framewrok
    vTaskDelete((TaskHandle_t)0); // delete this FreeRTOS task
#endif
}

//............................................................................
//! @public @memberof QActive
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    uint_fast16_t const qLen,
    void * const stkSto,
    uint_fast16_t const stkSize,
    void const * const par)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // precondition:
    // - queue storage must be provided
    // - queue size must be provided
    // - stack storage must be provided
    // - stack size must be provided
    Q_REQUIRE_INCRIT(100,
        (qSto != (QEvtPtr *)0) && (qLen > 0U)
        && (stkSto != (void *)0) && (stkSize > 0U));
    QF_CRIT_EXIT();

    // create FreeRTOS message queue
    me->eQueue = xQueueCreateStatic(
        (UBaseType_t)qLen,     // length of the queue
        (UBaseType_t)sizeof(QEvtPtr), // item size
        (uint8_t *)qSto,       // storage buffer
        &me->osObject);        // static queue buffer
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(110, me->eQueue != (QueueHandle_t)0);
    QF_CRIT_EXIT();

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = 0U; // preemption-threshold (not used for AO registration)
    QActive_register_(me); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    (*me->super.vptr->init)(&me->super, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // task name provided by the user in QActive_setAttr() or default name
    char const *taskName = (me->thread.pxDummy1 != (void *)0)
                             ? (char const *)me->thread.pxDummy1
                             : (char const *)"AO";

    // The FreeRTOS priority of the AO thread can be specified in two ways:
    //
    // 1. Implicitly based on the AO's priority (by the formula specified
    //    in the macro FREERTOS_TASK_PRIO(), see qp_port.h). This option
    //    is chosen, when the higher-byte of the prioSpec parameter is set
    //    to zero.
    //
    // 2. Explicitly as the higher-byte of the prioSpec parameter.
    //    This option is chosen when the prioSpec parameter is not-zero.
    //    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
    //    as 10 and FreeRTOS priority as 5.
    //
    //    NOTE: The explicit FreeRTOS priority is NOT sanity-checked,
    //    so it is the responsibility of the application to ensure that
    //    it is consistent with the AO's priority. An example of
    //    inconsistent setting would be assigning FreeRTOS priorities that
    //    would result in a different relative prioritization of AO's threads
    //    than indicated by the AO priorities assigned.
    //
    UBaseType_t freertos_prio = (prioSpec >> 8U);
    if (freertos_prio == 0U) {
        freertos_prio = FREERTOS_TASK_PRIO(me->prio);
    }

    // statically create the FreeRTOS task for the AO
    TaskHandle_t task = xTaskCreateStatic(
        &task_function, // the task function
        taskName ,      // the name of the task
        stkSize/sizeof(portSTACK_TYPE), // stack length
        (void *)me,     // the 'pvParameters' parameter
        freertos_prio,  // FreeRTOS priority
        (StackType_t *)stkSto,        // stack storage
        &me->thread);   // task buffer

    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(120, task != (TaskHandle_t)0);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(task);
#endif
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive_stop(QActive * const me) {
    QActive_unsubscribeAll(me); // unsubscribe from all events
    me->eQueue = (QueueHandle_t)0; // stop thread (see task_function())
}
#endif
//............................................................................
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // this function must be called before QActive_start(),
    // which implies that me->thread.pxDummy1 must not be used yet;
    Q_REQUIRE_INCRIT(150, me->thread.pxDummy1 == (void *)0);
    switch (attr1) {
        case TASK_NAME_ATTR:
            // temporarily store the name, cast 'const' away
            me->thread.pxDummy1 = (void *)attr2;
            break;
        // ...
        default:
            break;
    }
    QF_CRIT_EXIT();
}

//============================================================================
//! @private @memberof QActive
bool QActive_post_(QActive * const me, QEvt const * const e,
                   uint_fast16_t const margin, void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != (QEvt *)0);

    // the number of free slots available in the FreeRTOS queue
    uint_fast16_t const nFree =
        (uint_fast16_t)FREERTOS_QUEUE_GET_FREE(me);

    // required margin available?
    bool status = false; // assume that event cannot be posted
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) { // free entries available in the queue?
            status = true; // can post
        }
        else { // no free entries available
            // The queue overflows, but QF_NO_MARGIN indicates that
            // the "event delivery guarantee" is required.
            Q_ERROR_INCRIT(210); // must be able to post the event
        }
    }
    else if (nFree > (QEQueueCtr)margin) { // enough free entries?
        status = true; // can post
    }
    else {
        // empty
    }

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(205, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    if (status) { // can post the event?
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, me->prio)
            QS_TIME_PRE();        // timestamp
            QS_OBJ_PRE(sender);   // the sender object
            QS_SIG_PRE(e->sig);   // the signal of the event
            QS_OBJ_PRE(me);       // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE((QEQueueCtr)nFree); // # free entries
            QS_EQC_PRE(0U);       // min # free entries (unknown)
        QS_END_PRE()

        QF_CRIT_EXIT();

        BaseType_t err = xQueueSendToBack(
                             me->eQueue, (void const *)&e, (TickType_t)0);

        QF_CRIT_ENTRY();
        // posting to the FreeRTOS message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(220, err == pdPASS);
        QF_CRIT_EXIT();

#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
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

        QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
        QF_gc(e); // recycle the event to avoid a leak
#endif // (QF_MAX_EPOOL > 0U)
    }

    return status;
}

//............................................................................
void QActive_postLIFO_(QActive * const me, QEvt const * const e) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the posted event must be be valid (which includes not NULL)
    Q_REQUIRE_INCRIT(300, e != (QEvt *)0);

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(305, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE((QEQueueCtr)FREERTOS_QUEUE_GET_FREE(me)); // # free
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    QF_CRIT_EXIT();

    BaseType_t err = xQueueSendToFront(
                         me->eQueue, (void const *)&e, (TickType_t)0);

    QF_CRIT_ENTRY();
    // LIFO posting to the FreeRTOS queue must succeed, see NOTE3
    Q_ASSERT_INCRIT(320, err == pdPASS);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
QEvt const *QActive_get_(QActive * const me) {
    QEvtPtr e;
    xQueueReceive(me->eQueue, (void *)&e, portMAX_DELAY);

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, me->prio)
        QS_TIME_PRE();          // timestamp
        QS_SIG_PRE(e->sig);     // the signal of this event
        QS_OBJ_PRE(me);         // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE((QEQueueCtr)FREERTOS_QUEUE_GET_FREE(me)); // # free
    QS_END_PRE()
    QS_CRIT_EXIT();

    return e;
}

//============================================================================
// The "FromISR" QP APIs for the FreeRTOS port...
bool QActive_postFromISR_(QActive * const me, QEvt const * const e,
                          uint_fast16_t const margin,
                          BaseType_t * const pxHigherPriorityTaskWoken,
                          void const * const sender)
{
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    Q_REQUIRE_INCRIT(400, e != (QEvt *)0);

    // the number of free slots available in the FreeRTOS queue
    uint_fast16_t const nFree =
        (uint_fast16_t)FREERTOS_QUEUE_GET_FREE(me);

    // required margin available?
    bool status = false; // assume that event cannot be posted
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) { // free entries available in the queue?
            status = true; // can post
        }
        else { // no free entries available
            // The queue overflows, but QF_NO_MARGIN indicates that
            // the "event delivery guarantee" is required.
            Q_ERROR_INCRIT(410); // must be able to post the event
        }
    }
    else if (nFree > (QEQueueCtr)margin) { // enough free entries?
        status = true; // can post
    }
    else {
        // empty
    }

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(405, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    if (status) { // can post the event?
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, me->prio)
            QS_TIME_PRE();        // timestamp
            QS_OBJ_PRE(sender);   // the sender object
            QS_SIG_PRE(e->sig);   // the signal of the event
            QS_OBJ_PRE(me);       // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE((QEQueueCtr)nFree); // # free entries
            QS_EQC_PRE(0U);       // min # free entries (unknown)
        QS_END_PRE()

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        BaseType_t err = xQueueSendToBackFromISR(me->eQueue,
                              (void const *)&e,
                              pxHigherPriorityTaskWoken);

        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        // posting to the FreeRTOS message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(420, err == pdPASS);
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
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

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#if (QF_MAX_EPOOL > 0U)
        QF_gcFromISR(e); // recycle the event to avoid a leak
#endif // (QF_MAX_EPOOL > 0U)
    }

    return status;
}
//............................................................................
void QActive_publishFromISR_(QEvt const * const e,
                          BaseType_t * const pxHigherPriorityTaskWoken,
                          void const * const sender)
{
    Q_REQUIRE_INCRIT(500, e != (QEvt *)0);

    QSignal const sig = e->sig;

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // the published signal must be within the configured range
    Q_REQUIRE_INCRIT(510, sig < (QSignal)QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_PUBLISH, 0U)
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
        Q_ASSERT_INCRIT(515, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e);
    }

    // make a local, modifiable copy of the subscriber set
    QPSet subscrSet = QActive_subscrList_[sig].set;

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    if (QPSet_notEmpty(&subscrSet)) { // any subscribers?
        // the highest-prio subscriber
        uint_fast8_t p = QPSet_findMax(&subscrSet);

        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        QActive *a = QActive_registry_[p];
        // the AO must be registered with the framework
        Q_ASSERT_INCRIT(520, a != (QActive *)0);

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        //QF_SCHED_LOCK_(p); // no scheduler locking in FreeRTOS
        do { // loop over all subscribers
            // QACTIVE_POST() asserts internally if the queue overflows
            QACTIVE_POST_FROM_ISR(a, e, pxHigherPriorityTaskWoken, sender);

            QPSet_remove(&subscrSet, p); // remove the handled subscriber
            if (QPSet_notEmpty(&subscrSet)) {  // still more subscribers?
                p = QPSet_findMax(&subscrSet); // highest-prio subscriber

                uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

                a = QActive_registry_[p];
                // the AO must be registered with the framework
                Q_ASSERT_INCRIT(530, a != (QActive *)0);

                portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
            }
            else {
                p = 0U; // no more subscribers
            }
        } while (p != 0U);

        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        Q_ASSERT_INCRIT(590, p == 0U); // all subscribers processed
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        //QF_SCHED_UNLOCK_(); // no scheduler locking in FreeRTOS
    }

#if (QF_MAX_EPOOL > 0U)
    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
    QF_gcFromISR(e);
#endif // (QF_MAX_EPOOL > 0U)
}

//............................................................................
//! @private @memberof QTimeEvt
void QTimeEvt_tickFromISR_(uint_fast8_t const tickRate,
                          BaseType_t * const pxHigherPriorityTaskWoken,
                          void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    Q_REQUIRE_INCRIT(600, tickRate < Q_DIM(QTimeEvt_timeEvtHead_));

    QTimeEvt *prev = &QTimeEvt_timeEvtHead_[tickRate];

#ifdef Q_SPY
    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        ++prev->ctr;
        QS_TEC_PRE(prev->ctr);   // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()
#endif // def Q_SPY

    // scan the linked-list of time events at this rate...
    while (true) {
        Q_ASSERT_INCRIT(610, prev != (QTimeEvt *)0); // sanity check

        QTimeEvt *te = prev->next; // advance down the time evt. list

        if (te == (QTimeEvt *)0) { // end of the list?
            // NO any new time events armed since the last QTimeEvt_tick_()?
            if (QTimeEvt_timeEvtHead_[tickRate].act == (void *)0) {
                break; // terminate the for-loop
            }

            prev->next = (QTimeEvt*)QTimeEvt_timeEvtHead_[tickRate].act;
            QTimeEvt_timeEvtHead_[tickRate].act = (void *)0;

            te = prev->next; // switch to the new list
        }

        // the time event 'te' must be valid
        Q_ASSERT_INCRIT(640, te != (QTimeEvt *)0);

        QTimeEvtCtr ctr = te->ctr; // move volatile into temporary

        if (ctr == 0U) { // time event scheduled for removal?
            prev->next = te->next;

            // mark time event 'te' as NOT linked
            te->flags &= (uint8_t)(~QTE_FLAG_IS_LINKED & 0xFFU);
            // do NOT advance the prev pointer

            // exit crit. section to reduce latency
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        else if (ctr == 1U) { // is time event about to expire?
            QActive * const act = (QActive *)te->act;
            prev = QTimeEvt_expire_(te, prev, act, tickRate);

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // QACTIVE_POST_FROM_ISR() asserts if the queue overflows
            QACTIVE_POST_FROM_ISR(act, &te->super,
                                  pxHigherPriorityTaskWoken,
                                  sender);
        }
        else { // time event keeps timing out
            --ctr; // decrement the tick counter
            te->ctr = ctr; // update the original
            prev = te; // advance to this time event

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        // re-enter crit. section to continue the loop
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    }

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}

//............................................................................
QEvt *QF_newXFromISR_(uint_fast16_t const evtSize,
                      uint_fast16_t const margin, enum_t const sig)
{
    // find the pool number that fits the requested event size...
    uint8_t poolNum = 0U; // zero-based poolNum initially
    for (; poolNum < QF_priv_.maxPool_; ++poolNum) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[poolNum])) {
            break;
        }
    }

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // precondition:
    // - cannot run out of registered pools
    Q_REQUIRE_INCRIT(700, poolNum < QF_priv_.maxPool_);

    ++poolNum; // convert to 1-based poolNum

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    // get event e (port-dependent)...
    QEvt *e;
#ifdef Q_SPY
    e = QMPool_getFromISR(&QF_priv_.ePool_[poolNum - 1U],
                  ((margin != QF_NO_MARGIN) ? margin : 0U),
                  (uint_fast8_t)QS_EP_ID + poolNum);
#else
    e = QMPool_getFromISR(&QF_priv_.ePool_[poolNum - 1U],
                      ((margin != QF_NO_MARGIN) ? margin : 0U), 0U);
#endif

    if (e != (QEvt *)0) { // was e allocated correctly?
        e->sig      = (QSignal)sig; // set the signal
        e->poolNum_ = poolNum;
        e->refCtr_  = 0U;

#ifdef Q_SPY
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        QS_BEGIN_PRE(QS_QF_NEW, (uint_fast8_t)QS_EP_ID + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
#endif // def Q_SPY
    }
    else { // event was not allocated
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        // This assertion means that the event allocation failed,
        // and this failure cannot be tolerated. The most frequent
        // reason is an event leak in the application.
        Q_ASSERT_INCRIT(720, margin != QF_NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_NEW_ATTEMPT,
                (uint_fast8_t)QS_EP_ID + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    }

    // the returned event e is guaranteed to be valid (not NULL)
    // if we can't tolerate failed allocation
    return e;
}
//............................................................................
void QF_gcFromISR(QEvt const * const e) {
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    Q_REQUIRE_INCRIT(800, e != (QEvt *)0);

    uint8_t const poolNum = e->poolNum_;

    if (poolNum != 0U) { // is it a pool event (mutable)?

        if (e->refCtr_ > 1U) { // isn't this the last reference?

            QS_BEGIN_PRE(QS_QF_GC_ATTEMPT,
                    (uint_fast8_t)QS_EP_ID + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            Q_ASSERT_INCRIT(805, e->refCtr_ > 0U);
            QEvt_refCtr_dec_(e); // decrement the ref counter

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        else { // this is the last reference to this event, recycle it

            QS_BEGIN_PRE(QS_QF_GC,
                    (uint_fast8_t)QS_EP_ID + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            // pool number must be in range
            Q_ASSERT_INCRIT(810, (poolNum <= QF_priv_.maxPool_)
                                  && (poolNum <= QF_MAX_EPOOL));
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // NOTE: casting 'const' away is legit because 'e' is a pool event
#ifdef Q_SPY
            // cast 'const' away in (QEvt *)e is OK because it's a pool event
            QMPool_putFromISR(&QF_priv_.ePool_[poolNum - 1U], (QEvt *)e,
                              (uint_fast8_t)QS_EP_ID + e->poolNum_);
#else
            QMPool_putFromISR(&QF_priv_.ePool_[poolNum - 1U], (QEvt *)e, 0U);
#endif
        }
    }
    else {
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    }
}
//............................................................................
void *QMPool_getFromISR(QMPool * const me, uint_fast16_t const margin,
                        uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // get volatile into temporaries
    void * *pfb = me->freeHead; // pointer to free block
    QMPoolCtr nFree = me->nFree;

    // have more free blocks than the requested margin?
    if (nFree > (QMPoolCtr)margin) {
        Q_ASSERT_INCRIT(910, pfb != (void**)0);

        void ** const pfb_next = pfb[0]; // fast temporary

        --nFree; // one less free block
        if (nFree == 0U) { // is the pool becoming empty?
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_INCRIT(920, pfb_next == (void**)0);

            me->nFree = 0U; // no more free blocks
            me->nMin = 0U;  // remember that the pool got empty
        }
        else { // the pool is NOT empty

            // the next free-block pointer must be in range
            Q_ASSERT_INCRIT(930,
                (me->start <= pfb_next) && (pfb_next <= me->end));

            me->nFree = nFree; // update the original
            if (me->nMin > nFree) { // is this the new minimum?
                me->nMin = nFree; // remember the minimum so far
            }
        }

        me->freeHead = pfb_next; // set the head to the next free block

        // change the allocated block contents so that it is different
        // than a free block inside the pool.
        pfb[0] = &me->end[1]; // invalid location beyond the end

        QS_BEGIN_PRE(QS_QF_MPOOL_GET, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(me);        // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(me->nMin);  // min # free blocks ever in the pool
        QS_END_PRE()
    }
    else { // don't have enough free blocks at this point
        pfb = (void**)0;

        QS_BEGIN_PRE(QS_QF_MPOOL_GET_ATTEMPT, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(me);        // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(margin);    // the requested margin
        QS_END_PRE()
    }

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    return (void *)pfb; // return the block or NULL pointer to the caller
}
//............................................................................
void QMPool_putFromISR(QMPool * const me, void *block,
                       uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    void * * const pfb = (void * *)block; // pointer to free block

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // get volatile into temporaries
    void * * const freeHead = me->freeHead;
    QMPoolCtr nFree = me->nFree;

    Q_REQUIRE_INCRIT(1000, nFree < me->nTot);
    Q_REQUIRE_INCRIT(1010, (me->start <= pfb) && (pfb <= me->end));

    ++nFree; // one more free block in this pool

    me->freeHead = pfb; // set as new head of the free list
    me->nFree = nFree;
    pfb[0] = freeHead; // link into the list

    QS_BEGIN_PRE(QS_QF_MPOOL_PUT, qsId)
        QS_TIME_PRE();         // timestamp
        QS_OBJ_PRE(me);        // this memory pool
        QS_MPC_PRE(nFree);     // the # free blocks in the pool
    QS_END_PRE()

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}

//============================================================================
// NOTE3:
// The event posting to FreeRTOS message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//

