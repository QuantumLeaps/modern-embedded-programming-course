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
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // QS port
    #include "qs_pkg.h"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

Q_DEFINE_THIS_MODULE("qep_hsm")

// maximum depth of state nesting in a QHsm (including the top level),
// must be >= 3
#define QHSM_MAX_NEST_DEPTH_  6

static QEvt const l_reservedEvt_[4] = {
    QEVT_INITIALIZER(Q_EMPTY_SIG),
    QEVT_INITIALIZER(Q_ENTRY_SIG),
    QEVT_INITIALIZER(Q_EXIT_SIG),
    QEVT_INITIALIZER(Q_INIT_SIG)
};

//============================================================================
//! @cond INTERNAL

// internal helper macro to pass a reserved event into the state handler
#define QHSM_RESERVED_EVT_(state_, sig_) \
    ((*(state_))(me, &l_reservedEvt_[(sig_)]))

#ifdef Q_SPY
// helper macro to trace state action (entry/exit)
#define QS_STATE_ACT_(rec_, state_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_OBJ_PRE(me);        \
        QS_FUN_PRE(state_);    \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to top-most init
#define QS_TOP_INIT_(rec_, trg_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_TIME_PRE();         \
        QS_OBJ_PRE(me);        \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to trace transition segment
#define QS_TRAN_SEG_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_OBJ_PRE(me);        \
        QS_FUN_PRE(src_);      \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to trace transition action
#define QS_TRAN_ACT_(rec_, state_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_SIG_PRE(e->sig);    \
        QS_OBJ_PRE(me);        \
        QS_FUN_PRE(state_);    \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to trace transition begin/end
#define QS_TRAN0_(rec_, trg_)  \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_TIME_PRE();         \
        QS_SIG_PRE(e->sig);    \
        QS_OBJ_PRE(me);        \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to trace regular transition
#define QS_TRAN_END_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_TIME_PRE();         \
        QS_SIG_PRE(e->sig);    \
        QS_OBJ_PRE(me);        \
        QS_FUN_PRE(src_);      \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

#else
#define QS_STATE_ACT_(rec_, state_)     ((void)0)
#define QS_TOP_INIT_(rec_, trg_)        ((void)0)
#define QS_TRAN_SEG_(rec_, src_, trg_)  ((void)0)
#define QS_TRAN_ACT_(rec_, state_)      ((void)0)
#define QS_TRAN0_(rec_, trg_)           ((void)0)
#define QS_TRAN_END_(rec_, src_, trg_)  ((void)0)
#endif

//! @endcond

//============================================================================
//! @private @memberof QHsm
static int_fast8_t QHsm_tran_simple_(
    QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId);

//! @private @memberof QHsm
static int_fast8_t QHsm_tran_complex_(
    QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId);

//! @private @memberof QHsm
static void QHsm_enter_target_(
    QAsm * const me,
    QStateHandler * const path,
    int_fast8_t const depth,
    uint_fast8_t const qsId);

//............................................................................
//! @protected @memberof QHsm
void QHsm_ctor(QHsm * const me,
    QStateHandler const initial)
{
    static struct QAsmVtable const vtable = { // QAsm virtual table
        &QHsm_init_,
        &QHsm_dispatch_,
        &QHsm_isIn_
#ifdef Q_SPY
        ,&QHsm_getStateHandler_
#endif
    };
    // do not call the QAsm_ctor() here
    me->super.vptr      = &vtable;
    me->super.state.fun = Q_STATE_CAST(&QHsm_top);
    me->super.temp.fun  = initial;
}

//............................................................................
//! @protected @memberof QHsm
QState QHsm_top(QHsm const * const me,
    QEvt const * const e)
{
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(e);
    return Q_RET_IGNORED; // the top state ignores all events
}

//............................................................................
//! @private @memberof QHsm
void QHsm_init_(
    QAsm * const me,
    void const * const e,
    uint_fast8_t const qsId)
{
    // produce QS dictionary for QHsm_top()
#ifdef Q_SPY
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    bool isDone = true;
    if ((QS_priv_.flags & 0x01U) == 0U) {
        QS_priv_.flags |= 0x01U;
        isDone = false;
    }
    QS_CRIT_EXIT();
    if (!isDone) {
        QS_FUN_DICTIONARY(&QHsm_top);
    }
#else
    Q_UNUSED_PAR(qsId);
#endif // def Q_SPY

    QStateHandler const s = me->state.fun; // current state
    Q_REQUIRE_LOCAL(200, s == Q_STATE_CAST(&QHsm_top));
    Q_REQUIRE_LOCAL(210, me->temp.fun != Q_STATE_CAST(0));

    // execute the top-most initial tran.
    QState const r = (*me->temp.fun)(me, Q_EVT_CAST(QEvt));

    // the top-most initial tran. must be taken
    Q_ASSERT_LOCAL(240, r == Q_RET_TRAN);
    Q_ASSERT_LOCAL(250, me->temp.fun != Q_STATE_CAST(0));

    QS_TRAN_SEG_(QS_QEP_STATE_INIT, s, me->temp.fun);

    // drill down into the state hierarchy with initial transitions...
    QStateHandler path[QHSM_MAX_NEST_DEPTH_]; // tran. entry path array
    int_fast8_t ip = -1; // entry path index (one below [0])
    do {
        ++ip; // fixed loop bound
        Q_INVARIANT_LOCAL(260, ip < QHSM_MAX_NEST_DEPTH_);

        Q_ASSERT_LOCAL(270, me->temp.fun != Q_STATE_CAST(0));
        path[ip] = me->temp.fun; // store the entry path
        (void)QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);
    } while (me->temp.fun != s);

    QHsm_enter_target_(me, &path[0], ip, qsId);

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, path[0]);

    me->state.fun = path[0]; // change the current active state
#ifndef Q_UNSAFE
    // establish stable state configuration
    me->temp.uint = (uintptr_t)~me->state.uint;
#else
    Q_UNUSED_PAR(r);
#endif
}

//............................................................................
//! @private @memberof QHsm
void QHsm_dispatch_(
    QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    Q_INVARIANT_LOCAL(300, me->state.uint == (uintptr_t)(~me->temp.uint));

    Q_REQUIRE_LOCAL(310, e != (QEvt *)0);

    QStateHandler s = me->state.fun; // current state
    QS_CRIT_STAT
    QS_TRAN0_(QS_QEP_DISPATCH, s);

    // process the event hierarchically...
    QStateHandler path[QHSM_MAX_NEST_DEPTH_];
    path[0] = s; // save current state
    me->temp.fun = s;
    QState r;
    int_fast8_t ip = QHSM_MAX_NEST_DEPTH_;
    do {
        --ip; // fixed loop bound
        Q_INVARIANT_LOCAL(340, ip >= 0);

        s = me->temp.fun;
        path[ip] = s; // store the path to potential tran. source

        r = (*s)(me, e); // invoke state handler s

        if (r == Q_RET_UNHANDLED) { // unhandled due to a guard?
            QS_TRAN_ACT_(QS_QEP_UNHANDLED, s);
            r = QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG); // superstate of s
        }
    } while (r == Q_RET_SUPER);

    if (r >= Q_RET_TRAN) { // tran. (regular or history) taken?
        Q_ASSERT_LOCAL(350, me->temp.fun != Q_STATE_CAST(0));

#ifdef Q_SPY
        if (r == Q_RET_TRAN_HIST) { // tran. to history?
            QS_TRAN_SEG_(QS_QEP_TRAN_HIST, s, me->temp.fun);
        }
#endif // Q_SPY

        path[0] = me->temp.fun; // save tran. target

        // exit current state to tran. source...
        for (int_fast8_t iq = QHSM_MAX_NEST_DEPTH_ - 1; iq > ip; --iq) {
            // exit from path[iq]
            if (QHSM_RESERVED_EVT_(path[iq], Q_EXIT_SIG) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_EXIT, path[iq]);
            }
        }
        path[2] = s; // save tran. source

        // take the tran...
        ip = QHsm_tran_simple_(me, &path[0], qsId);
        if (ip < -1) { // not a simple tran.?
            ip = QHsm_tran_complex_(me, &path[0], qsId);
        }

        QHsm_enter_target_(me, &path[0], ip, qsId);
        QS_TRAN_END_(QS_QEP_TRAN, s, path[0]);
    }
#ifdef Q_SPY
    else if (r == Q_RET_HANDLED) {
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s);
    }
    else {
        QS_TRAN0_(QS_QEP_IGNORED, me->state.fun);
    }
#endif // Q_SPY

    me->state.fun = path[0]; // change the current active state
#ifndef Q_UNSAFE
    // establish stable state configuration
    me->temp.uint = (uintptr_t)~me->state.uint;
#endif
}

//............................................................................
//! @private @memberof QHsm
static int_fast8_t QHsm_tran_simple_(
    QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QStateHandler t = path[0];       // target
    QStateHandler const s = path[2]; // source
    int_fast8_t ip = 0; // assume to enter the target
    QS_CRIT_STAT

    // (a) check source==target (tran. to self)...
    if (s == t) {
        // exit source s (external tran. semantics)
        if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
        }
    }
    else {
        // find superstate of target
        QState const r = QHSM_RESERVED_EVT_(t, Q_EMPTY_SIG);
        Q_ASSERT_LOCAL(440, r == Q_RET_SUPER);
        Q_ASSERT_LOCAL(450, me->temp.fun != Q_STATE_CAST(0));
#ifdef Q_UNSAFE
        Q_UNUSED_PAR(r);
#endif

        // (b) check source==target->super...
        t = me->temp.fun;
        if (s != t) {
            // find superstate of source
            (void)QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG);

            // (c) check source->super==target->super...
            if (me->temp.fun == t) {
                // exit source s
                if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                }
            }
            // (d) check source->super==target...
            else if (me->temp.fun == path[0]) {
                // exit source s
                if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                }
                ip = -1; // do not enter the target
            }
            else {
                path[1] = t; // save the superstate of target
                ip = -2; // cause execution of QHsm_tran_complex_()
            }
        }
    }
    // # levels in path[] for QHsm_enter_target_()
    // or indication to call QHsm_tran_complex_()
    return ip;
}

//............................................................................
//! @private @memberof QHsm
static int_fast8_t QHsm_tran_complex_(
    QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QStateHandler s = path[2];             // source
    QStateHandler const ss = me->temp.fun; // source->super
    me->temp.fun = path[1];                // target->super
    int_fast8_t iq = 0; // assume that LCA is NOT found
    QState r;

    // (e) check rest of source == target->super->super...
    // and store the target entry path along the way
    int_fast8_t ip = 0; // entry path index (one below [1])
    do {
        ++ip; // fixed loop bound
        Q_INVARIANT_LOCAL(540, ip < QHSM_MAX_NEST_DEPTH_);

        path[ip] = me->temp.fun;
        r = QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);
        if (me->temp.fun == s) {
            iq = 1; // indicate that LCA is found
            break;
        }
    } while (r == Q_RET_SUPER);

    if (iq == 0) { // the LCA not found yet?
        QS_CRIT_STAT

        // exit source s
#ifndef Q_SPY
        (void)QHSM_RESERVED_EVT_(s, Q_EXIT_SIG);
#else
        if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
        }
#endif // def Q_SPY

        // (f) check the rest of
        // source->super... == target->super->super...
        s = ss; // source->super
        r = Q_RET_IGNORED; // assume that the LCA NOT found
        iq = ip; // outside for(;;) to comply with MC:2023 R14.2
        for (; iq >= 0; --iq) {
            if (s == path[iq]) { // is this the LCA?
                r = Q_RET_HANDLED; // indicate the LCA found
                ip = iq - 1; // do not enter the LCA
                break;
            }
        }

        if (r != Q_RET_HANDLED) { // the LCA still not found?
            // (g) check each source->super->...
            // for each target->super...
            int_fast8_t lbound = QHSM_MAX_NEST_DEPTH_;
            do {
                // exit from s
                if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                    // find superstate of s
                    (void)QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG);
                }
                s = me->temp.fun; // set to super of s
                // NOTE: loop bounded per invariant:540
                iq = ip; // outside for(;;) to comply with MC:2023 R14.2
                for (; iq >= 0; --iq) {
                    if (s == path[iq]) { // is this the LCA?
                        ip = iq - 1; // indicate not to enter the LCA
                        r = Q_RET_HANDLED; // cause break from outer loop
                        break;
                    }
                }

                --lbound; // fixed loop bound
                Q_INVARIANT_LOCAL(560, lbound >= 0);
            } while (r != Q_RET_HANDLED);
        }
    }
    // # levels in path[] for QHsm_enter_target_()
    return ip;
}

//............................................................................
//! @private @memberof QHsm
static void QHsm_enter_target_(
    QAsm * const me,
    QStateHandler * const path,
    int_fast8_t const depth,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QS_CRIT_STAT
    int_fast8_t ip = depth;
    // execute entry actions from LCA to tran target...
    for (; ip >= 0; --ip) {
        if (QHSM_RESERVED_EVT_(path[ip], Q_ENTRY_SIG) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);
        }
    }
    QStateHandler t = path[0]; // tran. target

    // drill into the target hierarchy with nested initial trans...
    int_fast8_t lbound = QHSM_MAX_NEST_DEPTH_ - 1;
    while (QHSM_RESERVED_EVT_(t, Q_INIT_SIG) == Q_RET_TRAN) {
        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(640, lbound >= 0);

        Q_ASSERT_LOCAL(650, me->temp.fun != Q_STATE_CAST(0));
        QS_TRAN_SEG_(QS_QEP_STATE_INIT, t, me->temp.fun);

        // find superstate of initial tran. target...
        ip = -1; // entry path index (one below [0])
        do {
            ++ip; // fixed loop bound
            Q_INVARIANT_LOCAL(660, ip < QHSM_MAX_NEST_DEPTH_);

            path[ip] = me->temp.fun; // store the entry path
            QState const r = QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);
            Q_ASSERT_LOCAL(680, r == Q_RET_SUPER);
#ifdef Q_UNSAFE
            Q_UNUSED_PAR(r);
#endif
        } while (me->temp.fun != t);

        // retrace the entry path in reverse (correct) order...
        for (; ip >= 0; --ip) {
            // enter path[ip]
            if (QHSM_RESERVED_EVT_(path[ip], Q_ENTRY_SIG) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);
            }
        }
        t = path[0]; // tran. target becomes the new source
    }
}

//............................................................................
//! @private @memberof QHsm
bool QHsm_isIn_(
    QAsm * const me,
    QStateHandler const stateHndl)
{
    Q_INVARIANT_LOCAL(700, me->state.uint == (uintptr_t)(~me->temp.uint));

    bool inState = false; // assume that this HSM is NOT in 'stateHndl'

    // scan the state hierarchy bottom-up
    QStateHandler s = me->state.fun;
    QState r;
    int_fast8_t lbound = QHSM_MAX_NEST_DEPTH_;
    do {
        if (s == stateHndl) { // do the states match?
            inState = true;  // 'true' means that match found
            break; // break out of the for-loop
        }
        r = QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG);
        s = me->temp.fun;

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(740, lbound >= 0);
    } while (r == Q_RET_SUPER);

    // restore the invariant (stable state configuration)
    me->temp.uint = (uintptr_t)~me->state.uint;

    return inState; // return the status
}

//............................................................................
#ifdef Q_SPY
//! @private @memberof QHsm
QStateHandler QHsm_getStateHandler_(QAsm * const me) {
    return me->state.fun;
}
#endif // def Q_SPY

//............................................................................
//! @public @memberof QHsm
QStateHandler QHsm_childState(QHsm * const me,
    QStateHandler const parentHndl)
{
#ifndef Q_UNSAFE
    bool isFound = false; // assume the child state NOT found
#endif

    QStateHandler child = me->super.state.fun; // start with current state
    me->super.temp.fun = child; // establish stable state configuration
    QState r = Q_RET_SUPER;
    int_fast8_t lbound = QHSM_MAX_NEST_DEPTH_;
    do {
        // have the parent of the current child?
        if (me->super.temp.fun == parentHndl) {
#ifndef Q_UNSAFE
            isFound = true; // indicate that child state was found
#endif
            break;
        }
        child = me->super.temp.fun;
        r = QHSM_RESERVED_EVT_(child, Q_EMPTY_SIG);

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(840, lbound >= 0);
    } while (r == Q_RET_SUPER);
    Q_ENSURE_LOCAL(890, isFound);

    // restore the invariant (stable state configuration)
    me->super.temp.uint = (uintptr_t)~me->super.state.uint;

    return child;
}

