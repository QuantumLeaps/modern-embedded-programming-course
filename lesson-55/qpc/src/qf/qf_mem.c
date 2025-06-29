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

Q_DEFINE_THIS_MODULE("qf_mem")

//............................................................................
//! @public @memberof QMPool
void QMPool_init(QMPool * const me,
    void * const poolSto,
    uint_fast32_t const poolSize,
    uint_fast16_t const blockSize)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(100, poolSto != (void *)0);

    me->freeHead = (void * *)poolSto;

    // find # free links in a memory block, see NOTE1
    me->blockSize = (QMPoolSize)(2U * sizeof(void *));
    uint_fast16_t inext = 2U; // # index of the next block
    while (me->blockSize < (QMPoolSize)blockSize) {
        me->blockSize += (QMPoolSize)sizeof(void *);
        ++inext;
    }

    // the pool buffer must fit at least one rounded-up block
    Q_ASSERT_INCRIT(110, poolSize >= me->blockSize);

    // start at the head of the free list
    void * *pfb = me->freeHead; // pointer to free block
    uint32_t nTot = 1U; // the last block already in the list

    // chain all blocks together in a free-list...
    for (uint_fast32_t size = poolSize - me->blockSize;
         size >= (uint_fast32_t)me->blockSize;
         size -= (uint_fast32_t)me->blockSize)
    {
        pfb[0] = &pfb[inext]; // set the next link to next free block
        pfb = pfb[0]; // advance to the next block
        ++nTot;       // one more free block in the pool
    }
    pfb[0] = (void * *)0; // the last link points to NULL

    // dynamic range check
#if (QF_MPOOL_CTR_SIZE == 1U)
    Q_ASSERT_INCRIT(190, nTot < 0xFFU);
#elif (QF_MPOOL_CTR_SIZE == 2U)
    Q_ASSERT_INCRIT(190, nTot < 0xFFFFU);
#endif

    me->nTot  = (QMPoolCtr)nTot;
    me->nFree = me->nTot; // all blocks are free
    me->start = (void * *)poolSto; // the original start this pool buffer
    me->end   = pfb;      // the last block in this pool
    me->nMin  = me->nTot; // the minimum # free blocks

    QF_CRIT_EXIT();
}

//............................................................................
//! @public @memberof QMPool
void * QMPool_get(QMPool * const me,
    uint_fast16_t const margin,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // get volatile into temporaries
    void * *pfb = me->freeHead; // pointer to free block
    QMPoolCtr nFree = me->nFree;

    // have more free blocks than the requested margin?
    if (nFree > (QMPoolCtr)margin) {
        Q_ASSERT_INCRIT(310, pfb != (void * *)0);

        void * * const pfb_next = pfb[0]; // fast temporary

        --nFree; // one less free block
        if (nFree == 0U) { // is the pool becoming empty?
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_INCRIT(320, pfb_next == (void * *)0);

            me->nFree = 0U; // no more free blocks
            me->nMin = 0U;  // remember that the pool got empty
        }
        else { // the pool is NOT empty

            // the next free-block pointer must be in range
            Q_ASSERT_INCRIT(330,
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
        pfb = (void * *)0;

        QS_BEGIN_PRE(QS_QF_MPOOL_GET_ATTEMPT, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(me);        // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(margin);    // the requested margin
        QS_END_PRE()
    }

    QF_CRIT_EXIT();

    return (void *)pfb; // return the block or NULL pointer to the caller
}

//............................................................................
//! @public @memberof QMPool
void QMPool_put(QMPool * const me,
    void * const block,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    void * * const pfb = (void * *)block; // pointer to free block

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // get volatile into temporaries
    void * * const freeHead = me->freeHead;
    QMPoolCtr nFree = me->nFree;

    Q_REQUIRE_INCRIT(400, nFree < me->nTot);
    Q_REQUIRE_INCRIT(410, (me->start <= pfb) && (pfb <= me->end));

    ++nFree; // one more free block in this pool

    me->freeHead = pfb; // set as new head of the free list
    me->nFree     = nFree;
    pfb[0]        = freeHead; // link into the list

    QS_BEGIN_PRE(QS_QF_MPOOL_PUT, qsId)
        QS_TIME_PRE();         // timestamp
        QS_OBJ_PRE(me);        // this memory pool
        QS_MPC_PRE(nFree);     // the # free blocks in the pool
    QS_END_PRE()

    QF_CRIT_EXIT();
}

//============================================================================
// NOTE1:
// The memory buffer for the pool is organized as an array of void* pointers
// (see void * data type). These pointers are used to form a linked-list
// of free blocks in the pool. The first location pfb[0] is the actual link.
// The second location pfb[1] is used in SafeQP as the redundant "duplicate
// storage" for the link at pfb[0]. Even though the "duplicate storage" is NOT
// used in this QP edition, the minimum number of number of void* pointers
// (void * data type) inside a memory block is still kept at 2 to maintain
// the same policy for sizing the memory blocks.
