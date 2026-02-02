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

Q_DEFINE_THIS_MODULE("qf_act")

// QP version string embedded in the binary image
char const QP_versionStr[24] = "QP/C " QP_VERSION_STR;

//----------------------------------------------------------------------------
//! @public @memberof QEvt
void QEvt_ctor(QEvt * const me, enum_t const sig) {
    me->sig      = (QSignal)sig;
    me->poolNum_ = 0x00U; // not a pool event
    me->refCtr_  = 0xE0U; // use as an "event marker"
}
//............................................................................
//! @public @memberof QEvt
QEvt *QEvt_init(QEvt * const me, uint8_t const dummy) {
    // initialize a dynamic event without parameters
    Q_UNUSED_PAR(dummy);
    return me;
}
//............................................................................
//! @private @memberof QEvt
void QEvt_refCtr_inc_(QEvt const * const me) {
    // NOTE: this function must be called *inside* a critical section

    // the event reference count must not exceed the number of AOs
    // in the system plus each AO possibly holding one event reference
    Q_REQUIRE_INCRIT(200, me->refCtr_ < (QF_MAX_ACTIVE + QF_MAX_ACTIVE));

    QEvt * const mut_me = (QEvt*)me; // cast 'const' away
    ++mut_me->refCtr_;
}
//............................................................................
//! @private @memberof QEvt
void QEvt_refCtr_dec_(QEvt const * const me) {
    // NOTE: this function must be called inside a critical section
    QEvt * const mut_me = (QEvt*)me; // cast 'const' away
    --mut_me->refCtr_;
}
