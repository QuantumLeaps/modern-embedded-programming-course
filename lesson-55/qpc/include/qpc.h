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
#ifndef QPC_H_
#define QPC_H_

#include "qp_port.h"      // QP port from the port directory
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY // software tracing enabled?
    #include "qs_port.h"  // QS/C port from the port directory
#else
    #include "qs_dummy.h" // QS/C dummy interface (inactive)
#endif

#ifndef QP_API_VERSION
    #define QP_API_VERSION 0
#endif // #ifndef QP_API_VERSION

// QP API compatibility layer...
//============================================================================
#if (QP_API_VERSION < 800)

#define QM_SUPER_SUB(host_)     error "submachines no longer supported"
#define QM_TRAN_EP(tatbl_)      error "submachines no longer supported"
#define QM_TRAN_XP(xp_, tatbl_) error "submachines no longer supported"

#ifdef QEVT_DYN_CTOR
//! @deprecated #QEVT_DYN_CTOR, please use #QEVT_PAR_INIT
#define QEVT_PAR_INIT
#endif

//! @deprecated plain 'char' is no longer forbidden in MISRA-C:2023
typedef char char_t;

//! @deprecated Macro for starting an Active Object.
//! Use QActive::QActive_start() instead.
#define QACTIVE_START(me_, prioSpec_, qSto_, qLen_, stkSto_, stkSize_, par_) \
    (QActive_start((QActive *)(me_), (prioSpec_), \
        (qSto_), (qLen_), (stkSto_), (stkSize_), (par_)))

//! @deprecated Macro for starting an eXtended Thread.
//! Use QXThread::QXThread_start() instead.
#define QXTHREAD_START(me_, prioSpec_, qSto_, qLen_, stkSto_, stkSize_, par_) \
    (QXThread_start((QXThread *)(me_), (prioSpec_), \
        (qSto_), (qLen_), (stkSto_), (stkSize_), (par_)))

//! @deprecated Assertion failure handler.
//! Use Q_onError() instead.
#define Q_onAssert(module_, id_) Q_onError(module_, id_)

//! @deprecated #Q_NASSERT preprocessor switch to disable QP assertions
#ifdef Q_NASSERT

    // #Q_UNSAFE now replaces the functionality of Q_NASSERT
    #define Q_UNSAFE

    //! @deprecated general purpose assertion with user-specified ID
    //! number that **always** evaluates the `expr_` expression.
    #define Q_ALLEGE_ID(id_, expr_) ((void)(expr_))

#elif defined Q_UNSAFE

    //! @deprecated general purpose assertion with user-specified ID
    //! number that **always** evaluates the `expr_` expression.
    #define Q_ALLEGE_ID(id_, expr_) ((void)(expr_))

#else // QP FuSa Subsystem enabled

    //! @deprecated general purpose assertion with user-specified ID
    //! number that **always** evaluates the `expr_` expression.
    //! @note
    //! The use of this macro is no longer recommended.
    #define Q_ALLEGE_ID(id_, expr_) if (!(expr_)) { \
        QF_CRIT_STAT \
        QF_CRIT_ENTRY(); \
        Q_onError(&Q_this_module_[0], (id_)); \
        QF_CRIT_EXIT(); \
    } else ((void)0)

#endif

//! @deprecated general purpose assertion without ID number
//! that **always** evaluates the `expr_` expression.
//! Instead of ID number, this macro is based on the standard
//! `__LINE__` macro.
//!
//! @note The use of this macro is no longer recommended.
#define Q_ALLEGE(expr_)         Q_ALLEGE_ID(__LINE__, (expr_))

//! Static (compile-time) assertion.
//! @deprecated
//! Use Q_ASSERT_STATIC() or better yet `_Static_assert()` instead.
#define Q_ASSERT_COMPILE(expr_) Q_ASSERT_STATIC(expr_)

//! @static @public @memberof QF
//! @deprecated
static inline void QF_psInit(
    QSubscrList * const subscrSto,
    enum_t const maxSignal)
{
    QActive_psInit(subscrSto, maxSignal);
}

//! @deprecated instead use: QASM_INIT()
#define QHSM_INIT(me_, par_, qsId_)   QASM_INIT((me_), (par_), (qsId_))

//! @deprecated instead use: QASM_DISPATCH()
#define QHSM_DISPATCH(me_, e_, qsId_) QASM_DISPATCH((me_), (e_), (qsId_))

//! @deprecated instead use: QASM_IS_IN()
#define QHsm_isIn(me_, state_)        QASM_IS_IN((QAsm *)(me_), (state_))

#define QF_PUBLISH(e_, sender_)       QACTIVE_PUBLISH((e_), (sender_))
#define QF_TICK_X(tickRate_, sender_) QTIMEEVT_TICK_X((tickRate_), (sender_))
#define QF_TICK(sender_)              QTIMEEVT_TICK(sender_)

//! @deprecated instead use: QActive::QActive_getQueueMin()
#define QF_getQueueMin(prio_)   (QActive_getQueueMin((prio_)))

#endif // QP_API_VERSION < 800

#endif // QPC_H_
