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
// see NOTE0
// define __TARGET_FPU_VFP symbol depending on the compiler...
#if defined (__CC_ARM)          // ARM Compiler
    // in ARM Compiler __TARGET_FPU_VFP is a pre-defined symbol
#elif defined (__ICCARM__)      // IAR Compiler
    #if defined __ARMVFP__
        #define __TARGET_FPU_VFP 1
    #endif
#elif defined (__GNUC__)        // GNU Compiler
    #if defined (__VFP_FP__) && !defined(__SOFTFP__)
        #define __TARGET_FPU_VFP 1
    #endif
#endif

//============================================================================
static void task_main(void *pVoid);  // prototype
static void task_main(void *pVoid) { // embOS task signature
    QActive * const act = (QActive *)pVoid;

#ifdef __TARGET_FPU_VFP
    // does the task use the FPU? see NOTE1
    if ((act->osObject & TASK_USES_FPU) != 0U) {
        OS_ExtendTaskContext_VFP();
    }
#endif  // __TARGET_FPU_VFP

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

    // the number of free slots available in the embOS queue
    uint_fast16_t const nFree =
        (uint_fast16_t)(me->eQueue.maxMsg - me->eQueue.nofMsg);

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

        QF_CRIT_EXIT(); // exit crit.sect. before calling Zephyr API

        // post the event to the embOS event queue, see NOTE3
        status = (OS_MAILBOX_Put(&me->eQueue, (OS_CONST_PTR void *)&e) == '\0');
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
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
        QS_EQC_PRE(me->eQueue.maxMsg - me->eQueue.nofMsg); // # free entries
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

    char err = OS_MAILBOX_PutFront(&me->eQueue, (OS_CONST_PTR void *)&e);

#ifndef Q_UNSAFE
    QF_CRIT_ENTRY();
    // LIFO posting to embOS mailbox must succeed, see NOTE3
    Q_ASSERT_INCRIT(230, err == '\0');
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
    OS_MAILBOX_GetBlocked(&me->eQueue, (void *)&e);

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, me->prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(me);      // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
        QS_EQC_PRE(me->eQueue.maxMsg - me->eQueue.nofMsg); // # free entries
    QS_END_PRE()
    QS_CRIT_EXIT();

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
    // create the embOS message box for the AO
    OS_MAILBOX_Create(&me->eQueue,
                (OS_U16)sizeof(QEvtPtr), // item size
                (OS_UINT)qLen,
                (void *)&qSto[0]);

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = 0U; // preemption-threshold (not used for AO registration)
    QActive_register_(me); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    QASM_INIT(me, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // embOS priority, see NOTE1
    OS_PRIO embos_prio = (OS_PRIO)(prioSpec >> 8U);
    if (embos_prio == 0U) {
        embos_prio = (OS_PRIO)me->prio;
    }

    // create an embOS task for the AO...
    OS_TASK_CreateEx(&me->thread,
#if (OS_TRACKNAME != 0)
        me->thread.Name, // the configured task name
#elif
        "AO",            // a generic AO task name
#endif
        embos_prio,      // embOS priority
        &task_main,
        (void OS_STACKPTR *)stkSto,
        (OS_UINT)stkSize,
        0U, // no AOs at the same prio
        (void *)me);
}
//............................................................................
//! @public @memberof QActive
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    // NOTE: this function must be called *before* QActive_start(),
    // which implies that me->thread.Name must not be used yet;
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    switch (attr1) {
        case TASK_NAME_ATTR: {
#if (OS_TRACKNAME != 0)
            Q_ASSERT_INCRIT(510, me->thread.Name == (char *)0);
            me->thread.Name = (char const *)attr2;
#endif
            break;
        }
        case TASK_USES_FPU:
            me->osObject = attr1;
            break;
        // ...
        default:
            break;
    }
    QF_CRIT_EXIT();
}

//============================================================================

//............................................................................
void QF_init(void) {
    QTimeEvt_init(); // initialize QTimeEvts
    OS_InitKern();   // initialize embOS
    OS_InitHW();     // initialize the hardware used by embOS
}
//............................................................................
int_t QF_run(void) {
    QF_onStartup(); // QF callback

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();
#endif // Q_SPY

    OS_Start(); // start embOS multitasking

    return 0; // this unreachable return keeps the compiler happy
}
//............................................................................
void QF_stop(void) {
    QF_onCleanup(); // cleanup callback
}

//============================================================================
// NOTE0:
// In case of hardware-supported floating point unit (FPU), a task must
// preserve the FPU registers across the context switch. However, this
// additional overhead is necessary only for tasks that actually use the
// FPU. In this QP-embOS port, an active object task that uses the FPU is
// designated by the QF_TASK_USES_FPU attribute, which can be set with the
// QF_setEmbOsTaskAttr() function. The task attributes must be set *before*
// calling QActive_start(). The task attributes are saved in the
// QActive.osObject member.
//
// NOTE1:
// The embOS priority of the AO thread can be specified in two ways:
//
// A. Implicitly based on the AO's priority (embOS uses the same
//    priority numbering scheme than QP). This option is chosen when
//    the higher-byte of the prioSpec parameter is set to zero.
//
// B. Explicitly as the higher-byte of the prioSpec parameter.
//    This option is chosen when the prioSpec parameter is not-zero.
//    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
//    as 10 and embOS priority as 5.
//
// CAUTION: The explicit embOS priority is NOT sanity-checked, so it is the
// responsibility of the application to ensure that it is consistent with the
// QP priority. An example of inconsistent setting would be assigning embOS
// priorities that would result in a different relative prioritization of AOs
// than indicated by the QP priorities assigned to the AOs.
//
// NOTE2:
// In the embOS port, the generic function QActive_setAttr() is used to
// set the options for the embOS task options and task name.
// CAUTION: QActive_setAttr() needs to be called *before* QActive_start() for
// the given AO.
//
// NOTE3:
// The event posting to embOS mailbox occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
