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

//Q_DEFINE_THIS_MODULE("qf_qmact")

//............................................................................
//! @protected @memberof QMActive
void QMActive_ctor(QMActive * const me,
    QStateHandler const initial)
{
    // clear the whole QMActive object, so that the framework can start
    // correctly even if the startup code fails to clear the uninitialized
    // data (as is required by the C Standard).
    QF_bzero_(me, sizeof(*me));

    // NOTE: QActive inherits the QActvie class, but it calls the
    // constructor of the QMsm subclass. This is because QMActive inherits
    // the behavior from the QMsm subclass.
    QMsm_ctor((QMsm *)(me), initial);

    // NOTE: this vtable is identical as QMsm, but is provided
    // for the QMActive subclass to provide a UNIQUE vptr to distinguish
    // subclasses of QActive (e.g., in the debugger).
    static struct QAsmVtable const vtable = { // QMActive virtual table
        &QMsm_init_,
        &QMsm_dispatch_,
        &QMsm_isIn_
#ifdef Q_SPY
        ,&QMsm_getStateHandler_
#endif
    };
    me->super.super.vptr = &vtable; // hook vptr to QMActive vtable
}
