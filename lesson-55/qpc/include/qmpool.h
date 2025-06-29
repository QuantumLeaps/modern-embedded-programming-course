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
#ifndef QMPOOL_H_
#define QMPOOL_H_

#ifndef QF_MPOOL_SIZ_SIZE
    #define QF_MPOOL_SIZ_SIZE 2U
#endif
#ifndef QF_MPOOL_CTR_SIZE
    #define QF_MPOOL_CTR_SIZE 2U
#endif

#define QF_MPOOL_EL(evType_) struct { \
    void * sto_[((sizeof(evType_) - 1U) / sizeof(void *)) + \
                    (sizeof(evType_) < (2U * sizeof(void *)) ? 2U : 1U)]; \
}

#if (QF_MPOOL_SIZ_SIZE == 1U)
    typedef uint8_t QMPoolSize;
#elif (QF_MPOOL_SIZ_SIZE == 2U)
    typedef uint16_t QMPoolSize;
#elif (QF_MPOOL_SIZ_SIZE == 4U)
    typedef uint32_t QMPoolSize;
#else
    #error QF_MPOOL_SIZ_SIZE defined incorrectly, expected 1U, 2U, or 4U
#endif

#if (QF_MPOOL_CTR_SIZE == 1U)
    typedef uint8_t QMPoolCtr;
#elif (QF_MPOOL_CTR_SIZE == 2U)
    typedef uint16_t QMPoolCtr;
#elif (QF_MPOOL_CTR_SIZE == 4U)
    typedef uint32_t QMPoolCtr;
#else
    #error QF_MPOOL_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U
#endif

//============================================================================
//! @class QMPool
typedef struct {
    void * * start;             //!< @private @memberof QMPool
    void * * end;               //!< @private @memberof QMPool
    void * * volatile freeHead; //!< @private @memberof QMPool
    QMPoolSize blockSize;       //!< @private @memberof QMPool
    QMPoolCtr nTot;             //!< @private @memberof QMPool
    QMPoolCtr volatile nFree;   //!< @private @memberof QMPool
    QMPoolCtr nMin;             //!< @private @memberof QMPool
} QMPool;

//! @public @memberof QMPool
void QMPool_init(QMPool * const me,
    void * const poolSto,
    uint_fast32_t const poolSize,
    uint_fast16_t const blockSize);

//! @public @memberof QMPool
void * QMPool_get(QMPool * const me,
    uint_fast16_t const margin,
    uint_fast8_t const qsId);

//! @public @memberof QMPool
void QMPool_put(QMPool * const me,
    void * const block,
    uint_fast8_t const qsId);

#endif  // QMPOOL_H_
