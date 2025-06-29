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
#ifndef QEQUEUE_H_
#define QEQUEUE_H_

#ifndef QF_EQUEUE_CTR_SIZE
    #define QF_EQUEUE_CTR_SIZE 1U
#endif

#if (QF_EQUEUE_CTR_SIZE == 1U)
    typedef uint8_t QEQueueCtr;
#elif (QF_EQUEUE_CTR_SIZE == 2U)
    typedef uint16_t QEQueueCtr;
#else
    #error QF_EQUEUE_CTR_SIZE defined incorrectly, expected 1U or 2U
#endif

struct QEvt; // forward declaration

//============================================================================
//! @class QEQueue
typedef struct QEQueue {
    struct QEvt const * volatile frontEvt; //!< @private @memberof QEQueue
    struct QEvt const * * ring;            //!< @private @memberof QEQueue
    QEQueueCtr end;                        //!< @private @memberof QEQueue
    QEQueueCtr volatile head;              //!< @private @memberof QEQueue
    QEQueueCtr volatile tail;              //!< @private @memberof QEQueue
    QEQueueCtr volatile nFree;             //!< @private @memberof QEQueue
    QEQueueCtr nMin;                       //!< @private @memberof QEQueue
} QEQueue;

//! @public @memberof QEQueue
void QEQueue_init(QEQueue * const me,
    struct QEvt const  * * const qSto,
    uint_fast16_t const qLen);

//! @public @memberof QEQueue
bool QEQueue_post(QEQueue * const me,
    struct QEvt const * const e,
    uint_fast16_t const margin,
    uint_fast8_t const qsId);

//! @public @memberof QEQueue
void QEQueue_postLIFO(QEQueue * const me,
    struct QEvt const * const e,
    uint_fast8_t const qsId);

//! @public @memberof QEQueue
struct QEvt const * QEQueue_get(QEQueue * const me,
    uint_fast8_t const qsId);

//! @public @memberof QEQueue
static inline QEQueueCtr QEQueue_getNFree(QEQueue const * const me) {
    return me->nFree;
}

//! @public @memberof QEQueue
static inline QEQueueCtr QEQueue_getNMin(QEQueue const * const me) {
    return me->nMin;
}

//! @public @memberof QEQueue
static inline bool QEQueue_isEmpty(QEQueue const * const me) {
    return me->frontEvt == (struct QEvt *)0;
}

#endif // QEQUEUE_H_
