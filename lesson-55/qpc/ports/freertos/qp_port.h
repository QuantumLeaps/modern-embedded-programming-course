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
#ifndef QP_PORT_H_
#define QP_PORT_H_

#include <stdint.h>     // Exact-width types. WG14/N843 C99 Standard
#include <stdbool.h>    // Boolean type.      WG14/N843 C99 Standard
#include "qp_config.h"  // QP configuration from the application

// no-return function specifier (C11 Standard)
#define Q_NORETURN   _Noreturn void

// QActive event queue and thread types for FreeRTOS
#define QACTIVE_EQUEUE_TYPE     QueueHandle_t
#define QACTIVE_OS_OBJ_TYPE     StaticQueue_t
#define QACTIVE_THREAD_TYPE     StaticTask_t

// FreeRTOS requires the "FromISR" API in QP/C
#define QF_ISR_API              1

// QF interrupt disabling/enabling (task level)
#define QF_INT_DISABLE()        taskDISABLE_INTERRUPTS()
#define QF_INT_ENABLE()         taskENABLE_INTERRUPTS()

// QF critical section for FreeRTOS (task level), see NOTE2
#define QF_CRIT_STAT
#define QF_CRIT_ENTRY()         taskENTER_CRITICAL()
#define QF_CRIT_EXIT()          taskEXIT_CRITICAL()
#define QF_CRIT_EST()           taskENTER_CRITICAL()

// include files -------------------------------------------------------------
#include "FreeRTOS.h"  // FreeRTOS master include file, see NOTE3
#include "task.h"      // FreeRTOS task management
#include "queue.h"     // FreeRTOS queue management

#include "qequeue.h"   // QP event queue (for deferring events)
#include "qmpool.h"    // QP memory pool (for event pools)
#include "qp.h"        // QP platform-independent public interface


// the "FromISR" versions of the QF APIs, see NOTE3
#ifdef Q_SPY

#define QACTIVE_POST_FROM_ISR(me_, e_, pxHigherPrioTaskWoken_, sender_) \
    ((void)QActive_postFromISR_((me_), (e_), QF_NO_MARGIN,              \
                                (pxHigherPrioTaskWoken_), (sender_)))

#define QACTIVE_POST_X_FROM_ISR(me_, e_, margin_,                \
                                pxHigherPrioTaskWoken_, sender_) \
    (QActive_postFromISR_((me_), (e_), (margin_),                \
                          (pxHigherPrioTaskWoken_), (sender_)))

#define QACTIVE_PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
    (QActive_publishFromISR_((e_), (pxHigherPrioTaskWoken_),          \
                        (void const *)(sender_)))

#define QTIMEEVT_TICK_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, sender_) \
    (QTimeEvt_tickFromISR_((tickRate_), (pxHigherPrioTaskWoken_), (sender_)))

#else // ndef Q_SPY

#define QACTIVE_POST_FROM_ISR(me_, e_, pxHigherPrioTaskWoken_, dummy) \
    ((void)QActive_postFromISR_((me_), (e_), QF_NO_MARGIN,              \
                                (pxHigherPrioTaskWoken_), (void *)0))

#define QACTIVE_POST_X_FROM_ISR(me_, e_, margin_,                \
                                pxHigherPrioTaskWoken_, dummy) \
    (QActive_postFromISR_((me_), (e_), (margin_),                \
                          (pxHigherPrioTaskWoken_), (void *)0))

#define QACTIVE_PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
    (QActive_publishFromISR_((e_), (pxHigherPrioTaskWoken_),          \
                        (void *)0))

#define QTIMEEVT_TICK_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, dummy) \
    (QTimeEvt_tickFromISR_((tickRate_), (pxHigherPrioTaskWoken_), (void *)0))

#endif // Q_SPY

bool QActive_postFromISR_(QActive * const me, QEvt const * const e,
                          uint_fast16_t const margin,
                          BaseType_t * const pxHigherPriorityTaskWoken,
                          void const * const sender);

void QActive_publishFromISR_(QEvt const * const e,
                          BaseType_t * const pxHigherPriorityTaskWoken,
                          void const * const sender);

void QTimeEvt_tickFromISR_(uint_fast8_t const tickRate,
                          BaseType_t * const pxHigherPriorityTaskWoken,
                          void const * const sender);

#define QF_TICK_FROM_ISR(pxHigherPrioTaskWoken_, sender_) \
    QTIMEEVT_TICK_FROM_ISR(0U, pxHigherPrioTaskWoken_, sender_)

#ifdef QEVT_DYN_CTOR // Shall the ctor for the ::QEvt class be provided?

    #define Q_NEW_FROM_ISR(evtT_, sig_, ...)                  \
        (evtT_##_ctor((evtT_ *)QF_newXFromISR_(sizeof(evtT_), \
                      QF_NO_MARGIN, (sig_)), __VA_ARGS__))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_, ...) do { \
        (e_) = (evtT_ *)QF_newXFromISR_(sizeof(evtT_),           \
                                 (margin_), (sig_));             \
        if ((e_) != (evtT_ *)0) {                                \
            evtT_##_ctor((e_), __VA_ARGS__);                     \
        }                                                        \
     } while (false)

#else // no ::QEvt ctor

    #define Q_NEW_FROM_ISR(evtT_, sig_)                         \
        ((evtT_ *)QF_newXFromISR_((uint_fast16_t)sizeof(evtT_), \
                                  QF_NO_MARGIN, (sig_)))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_) ((e_) = \
        (evtT_ *)QF_newXFromISR_((uint_fast16_t)sizeof(evtT_), \
                                 (margin_), (sig_)))

#endif // QEVT_DYN_CTOR

void QF_gcFromISR(QEvt const * const e);

QEvt *QF_newXFromISR_(uint_fast16_t const evtSize,
                      uint_fast16_t const margin, enum_t const sig);

void *QMPool_getFromISR(QMPool * const me, uint_fast16_t const margin,
                        uint_fast8_t const qsId);
void QMPool_putFromISR(QMPool * const me, void *block,
                        uint_fast8_t const qsId);

enum FreeRTOS_TaskAttrs {
    TASK_NAME_ATTR
};

// FreeRTOS hooks prototypes (not provided by FreeRTOS)
#if (configUSE_IDLE_HOOK > 0)
    void vApplicationIdleHook(void);
#endif
#if (configUSE_TICK_HOOK > 0)
    void vApplicationTickHook(void);
#endif
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
#endif
#if (configSUPPORT_STATIC_ALLOCATION > 0)
    void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                       StackType_t **ppxIdleTaskStackBuffer,
                                       uint32_t *pulIdleTaskStackSize);
#endif

//============================================================================
// interface used only inside QF, but not in applications

#ifdef QP_IMPL
    #define FREERTOS_TASK_PRIO(qp_prio_) \
        ((UBaseType_t)((qp_prio_) + tskIDLE_PRIORITY))

    // FreeRTOS scheduler locking for QF_publish_() (task context only)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(prio_) (vTaskSuspendAll())
    #define QF_SCHED_UNLOCK_()    ((void)xTaskResumeAll())

    // native QF event pool customization
    #define QF_EPOOL_TYPE_        QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((uint_fast16_t)(p_).blockSize)
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
        ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
    #define QF_EPOOL_PUT_(p_, e_, qsId_) \
        (QMPool_put(&(p_), (e_), (qsId_)))

#endif // def QP_IMPL

//============================================================================
// NOTE2:
// The critical section definition applies only to the FreeRTOS "task level"
// APIs. The "FromISR" APIs are defined separately.
//
// NOTE3:
// The design of FreeRTOS requires using different APIs inside the ISRs
// (the "FromISR" variant) than at the task level. Accordingly, this port
// provides the "FromISR" variants for QP functions and "FROM_ISR" variants
// for QP macros to be used inside ISRs. ONLY THESE "FROM_ISR" VARIANTS
// ARE ALLOWED INSIDE ISRs AND CALLING THE TASK-LEVEL APIs IS AN ERROR.

#endif // QP_PORT_H_

