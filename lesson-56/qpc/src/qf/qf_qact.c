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
    #include "qs_pkg.h"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

Q_DEFINE_THIS_MODULE("qf_qact")

//! @static @private @memberof QActive
QActive * QActive_registry_[QF_MAX_ACTIVE + 1U];

//! @static @private @memberof QF
QF_Attr QF_priv_;

//----------------------------------------------------------------------------
//! @protected @memberof QActive
void QActive_ctor(QActive * const me,
    QStateHandler const initial)
{
    // NOTE: QActive indirectly inherits the abstract QAsm base class,
    // but it will delegate the state machine behavior to the QHsm class,
    // so the following initiaization is identical as in QHsm ctor:
    QHsm_ctor((QHsm *)(me), initial);

    // NOTE: this vtable is identical as QHsm, but is provided
    // for the QActive subclass to ensure a UNIQUE vptr to distinguish
    // subclasses of QActive (e.g., in the debugger).
    static struct QAsmVtable const vtable = { // QActive virtual table
        &QHsm_init_,
        &QHsm_dispatch_,
        &QHsm_isIn_,
        &QHsm_getStateHandler_
    };
    me->super.vptr = &vtable; // hook vptr to QActive vtable
}

//............................................................................
//! @private @memberof QActive
void QActive_register_(QActive * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    if (me->pthre == 0U) { // preemption-threshold not defined?
        me->pthre = me->prio; // apply the default
    }

    // AO's prio. must be in range
    Q_REQUIRE_INCRIT(100, (0U < me->prio) && (me->prio <= QF_MAX_ACTIVE));

    // the AO must NOT be registered already
    Q_REQUIRE_INCRIT(110, QActive_registry_[me->prio] == (QActive *)0);

    // the AO's prio. must not exceed the preemption threshold
    Q_REQUIRE_INCRIT(130, me->prio <= me->pthre);

#ifndef Q_UNSAFE
    uint8_t prev_thre = me->pthre;
    uint8_t next_thre = me->pthre;

    for (uint8_t p = me->prio - 1U; p > 0U; --p) {
        if (QActive_registry_[p] != (QActive *)0) {
            prev_thre = QActive_registry_[p]->pthre;
            break;
        }
    }
    for (uint8_t p = me->prio + 1U; p <= QF_MAX_ACTIVE; ++p) {
        if (QActive_registry_[p] != (QActive *)0) {
            next_thre = QActive_registry_[p]->pthre;
            break;
        }
    }

    // the preemption threshold of this AO must be between
    // preemption threshold of the previous AO and next AO
    Q_ASSERT_INCRIT(160,
        (prev_thre <= me->pthre) && (me->pthre <= next_thre));

#endif // Q_UNSAFE

    // register the AO at the QF-prio.
    QActive_registry_[me->prio] = me;

    QF_CRIT_EXIT();
}

//............................................................................
//! @private @memberof QActive
void QActive_unregister_(QActive * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    uint8_t const p = me->prio; // put AO's prio. in a temporary

    // AO's prio. must be in range
    Q_REQUIRE_INCRIT(210, (0U < p) && (p <= QF_MAX_ACTIVE));

    // this AO must be registered at prio. p
    Q_REQUIRE_INCRIT(230, me == QActive_registry_[p]);

    me->super.state.fun = Q_STATE_CAST(0); // invalidate the state
    QActive_registry_[p] = (QActive *)0; // free-up the prio. level

    QF_CRIT_EXIT();
}

//----------------------------------------------------------------------------
#ifndef QF_LOG2
uint_fast8_t QF_LOG2(QPSetBits const bitmask) {
    // look-up table for log2(0..15)
    static uint8_t const log2LUT[16] = {
        0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
        4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
    };
    uint_fast8_t n = 0U;
    QPSetBits x = bitmask;
    QPSetBits tmp; // temporary for modified bitmask parameter

#if (QF_MAX_ACTIVE > 16U)
    tmp = (x >> 16U);
    if (tmp != 0U) { // x > 2^16?
        n += 16U;
        x = tmp;
    }
#endif
#if (QF_MAX_ACTIVE > 8U)
    tmp = (x >> 8U);
    if (tmp != 0U) { // x > 2^8?
        n += 8U;
        x = tmp;
    }
#endif
    tmp = (x >> 4U);
    if (tmp != 0U) {  // x > 2^4?
        n += 4U;
        x = tmp;
    }
    // x is guaranteed to be in the 0..15 range for the look-up
    return (uint_fast8_t)(n + log2LUT[x]);
}
#endif // ndef QF_LOG2

//----------------------------------------------------------------------------
//! @public @memberof QPSet
void QPSet_setEmpty(QPSet * const me) {
    me->bits0 = 0U; // clear bitmask for elements 1..32
#if (QF_MAX_ACTIVE > 32)
    me->bits1 = 0U; // clear bitmask for elements 33..64
#endif
}
//............................................................................
//! @public @memberof QPSet
bool QPSet_isEmpty(QPSet const * const me) {
#if (QF_MAX_ACTIVE <= 32U)
    return (me->bits0 == 0U);  // check only bitmask for elements 1..32
#else
    return (me->bits0 == 0U)   // bitmask for elements 1..32 empty?
           ? (me->bits1 == 0U) // check bitmask for for elements 33..64
           : false;             // the set is NOT empty
#endif
}
//............................................................................
//! @public @memberof QPSet
bool QPSet_notEmpty(QPSet const * const me) {
#if (QF_MAX_ACTIVE <= 32U)
    return (me->bits0 != 0U);   // check only bitmask for elements 1..32
#else
    return (me->bits0 != 0U)    // bitmask for elements 1..32 empty?
           ? true                // the set is NOT empty
           : (me->bits1 != 0U); // check bitmask for for elements 33..64
#endif
}
//............................................................................
//! @public @memberof QPSet
bool QPSet_hasElement(QPSet const * const me, uint_fast8_t const n) {
#if (QF_MAX_ACTIVE <= 32U)
    // check the bit only in bitmask for elements 1..32
    return (me->bits0 & ((QPSetBits)1U << (n - 1U))) != 0U;
#else
    return (n <= 32U) // which group of elements (1..32 or 33..64)?
        ? ((me->bits0 & ((QPSetBits)1U << (n - 1U)))  != 0U)
        : ((me->bits1 & ((QPSetBits)1U << (n - 33U))) != 0U);
#endif
}
//............................................................................
//! @public @memberof QPSet
void QPSet_insert(QPSet * const me, uint_fast8_t const n) {
#if (QF_MAX_ACTIVE <= 32U)
    // set the bit only in bitmask for elements 1..32
    me->bits0 = (me->bits0 | ((QPSetBits)1U << (n - 1U)));
#else
    if (n <= 32U) { // set the bit in the bitmask for elements 1..32?
        me->bits0 = (me->bits0 | ((QPSetBits)1U << (n - 1U)));
    }
    else { // set the bit in the bitmask for for elements 33..64
        me->bits1 = (me->bits1 | ((QPSetBits)1U << (n - 33U)));
    }
#endif
}
//............................................................................
//! @public @memberof QPSet
void QPSet_remove(QPSet * const me, uint_fast8_t const n) {
#if (QF_MAX_ACTIVE <= 32U)
    // clear the bit only in bitmask for elements 1..32
    me->bits0 = (me->bits0 & (QPSetBits)(~((QPSetBits)1U << (n - 1U))));
#else
    if (n <= 32U) { // clear the bit in the bitmask for elements 1..32?
        (me->bits0 = (me->bits0 & ~((QPSetBits)1U << (n - 1U))));
    }
    else { // clear the bit in the bitmask for for elements 33..64
        (me->bits1 = (me->bits1 & ~((QPSetBits)1U << (n - 33U))));
    }
#endif
}
//............................................................................
//! @public @memberof QPSet
uint_fast8_t QPSet_findMax(QPSet const * const me) {
#if (QF_MAX_ACTIVE <= 32U)
    // check only the bitmask for elements 1..32
    return QF_LOG2(me->bits0);
#else
    return (me->bits1 != 0U) // bitmask for elements 32..64 not empty?
        ? (32U + QF_LOG2(me->bits1)) // 32 + log2(bits 33..64)
        : (QF_LOG2(me->bits0));      // log2(bits 1..32)
#endif
}
