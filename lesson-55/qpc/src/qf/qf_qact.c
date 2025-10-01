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

//............................................................................
//! @protected @memberof QActive
void QActive_ctor(QActive * const me,
    QStateHandler const initial)
{
    // clear the whole QActive object, so that the framework can start
    // correctly even if the startup code fails to clear the uninitialized
    // data (as is required by the C Standard).
    QF_bzero_(me, sizeof(*me));

    // NOTE: QActive inherits the abstract QAsm class, but it calls the
    // constructor of the QHsm subclass. This is because QActive inherits
    // the behavior from the QHsm subclass.
    QHsm_ctor((QHsm *)(me), initial);

    // NOTE: this vtable is identical as QHsm, but is provided
    // for the QActive subclass to provide a UNIQUE vptr to distinguish
    // subclasses of QActive (e.g., in the debugger).
    static struct QAsmVtable const vtable = { // QActive virtual table
        &QHsm_init_,
        &QHsm_dispatch_,
        &QHsm_isIn_
#ifdef Q_SPY
        ,&QHsm_getStateHandler_
#endif
    };
    me->super.vptr = &vtable; // hook vptr to QActive vtable
}

//............................................................................
//! @private @memberof QActive
void QActive_register_(QActive * const me) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    uint8_t p = me->prio;
    if (me->pthre == 0U) { // preemption-threshold not defined?
        me->pthre = p; // apply the default
    }

    Q_REQUIRE_INCRIT(100, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(110, QActive_registry_[p] == (QActive *)0);
    Q_REQUIRE_INCRIT(130, p <= me->pthre);

#ifndef Q_UNSAFE
    uint8_t prev_thre = me->pthre;
    uint8_t next_thre = me->pthre;

    for (p = p - 1U; p > 0U; --p) {
        if (QActive_registry_[p] != (QActive *)0) {
            prev_thre = QActive_registry_[p]->pthre;
            break;
        }
    }
    for (p = me->prio + 1U; p <= QF_MAX_ACTIVE; ++p) {
        if (QActive_registry_[p] != (QActive *)0) {
            next_thre = QActive_registry_[p]->pthre;
            break;
        }
    }

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

    uint8_t const p = me->prio;
    Q_REQUIRE_INCRIT(210, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(230, QActive_registry_[p] == me);

    me->super.state.fun = Q_STATE_CAST(0); // invalidate the state
    QActive_registry_[p] = (QActive *)0; // free-up the prio. level

    QF_CRIT_EXIT();
}
