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
#define QS_STATE_ACT_(rec_, state_)    \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_OBJ_PRE(me);                \
        QS_FUN_PRE(state_);            \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to top-most init
#define QS_TOP_INIT_(rec_, trg_)       \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_TIME_PRE();                 \
        QS_OBJ_PRE(me);                \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to trace transition segment
#define QS_TRAN_SEG_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_OBJ_PRE(me);                \
        QS_FUN_PRE(src_);              \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to trace transition action
#define QS_TRAN_ACT_(rec_, state_)     \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_SIG_PRE(e->sig);            \
        QS_OBJ_PRE(me);                \
        QS_FUN_PRE(state_);            \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to trace transition begin/end
#define QS_TRAN0_(rec_, trg_)          \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_TIME_PRE();                 \
        QS_SIG_PRE(e->sig);            \
        QS_OBJ_PRE(me);                \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to trace regulsr transition
#define QS_TRAN_END_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_TIME_PRE();                 \
        QS_SIG_PRE(e->sig);            \
        QS_OBJ_PRE(me);                \
        QS_FUN_PRE(src_);              \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
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
//! @private @memberof QHsm
void QHsm_init_(
    QAsm * const me,
    void const * const e,
    uint_fast8_t const qsId)
{
    QF_CRIT_STAT

    // produce QS dictionary for QHsm_top()
#ifdef Q_SPY
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

    QStateHandler t = me->state.fun;

    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(200, (me->vptr != (struct QAsmVtable *)0)
        && (me->temp.fun != Q_STATE_CAST(0))
        && (t == Q_STATE_CAST(&QHsm_top)));
    QF_CRIT_EXIT();

    // execute the top-most initial tran.
    QState const r = (*me->temp.fun)(me, Q_EVT_CAST(QEvt));

    QF_CRIT_ENTRY();
    // the top-most initial tran. must be taken
    Q_ASSERT_INCRIT(210, r == Q_RET_TRAN);
    QF_CRIT_EXIT();

    QS_TRAN_SEG_(QS_QEP_STATE_INIT, t, me->temp.fun);

    // drill down into the state hierarchy with initial transitions...
    QStateHandler path[QHSM_MAX_NEST_DEPTH_]; // tran. entry path array
    path[0] = me->temp.fun;
    (void)QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);

    int_fast8_t ip = 1; // tran. entry path index (also the loop bound)
    for (; (me->temp.fun != t) && (ip < QHSM_MAX_NEST_DEPTH_); ++ip) {
        path[ip] = me->temp.fun;
        (void)QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);
    }
    QF_CRIT_ENTRY();
    // must NOT be too many state nesting levels or "malformed" HSM
    Q_ASSERT_INCRIT(220, ip <= QHSM_MAX_NEST_DEPTH_);
    QF_CRIT_EXIT();

    me->temp.fun = path[0];
    QHsm_enter_target_(me, &path[0], ip - 1, qsId);
    t = path[0];

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, t);

    me->state.fun = t; // change the current active state
#ifdef Q_UNSAFE
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

    QStateHandler s = me->state.fun;
    QStateHandler t = s;
    QF_CRIT_STAT

    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300,
        (e != (QEvt *)0)
        && (s != Q_STATE_CAST(0)));
    QF_CRIT_EXIT();

    QS_TRAN0_(QS_QEP_DISPATCH, s);

    QState r = Q_RET_SUPER;

    // process the event hierarchically...
    me->temp.fun = s;
    int_fast8_t ip = QHSM_MAX_NEST_DEPTH_;
    // NOTE: ip is the fixed loop upper bound
    for (; ip > 0; --ip) {
        s = me->temp.fun;
        r = (*s)(me, e); // invoke state handler s

        if (r == Q_RET_UNHANDLED) { // unhandled due to a guard?
            QS_TRAN_ACT_(QS_QEP_UNHANDLED, s);
            r = QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG); // superstate of s
        }
        if (r != Q_RET_SUPER) { // event NOT "bubbled up"
            break;
        }
    }
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(310, ip > 0);
    QF_CRIT_EXIT();

    if (r >= Q_RET_TRAN) { // tran. (regular or history) taken?
#ifdef Q_SPY
        if (r == Q_RET_TRAN_HIST) { // tran. to history?
            QS_TRAN_SEG_(QS_QEP_TRAN_HIST, s, me->temp.fun);
        }
#endif // Q_SPY

        QStateHandler path[QHSM_MAX_NEST_DEPTH_];
        path[0] = me->temp.fun; // tran. target
        path[1] = t; // current state
        path[2] = s; // tran. source

        // exit current state to tran. source s...
        while (t != s) {
            // exit from t
            if (QHSM_RESERVED_EVT_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_EXIT, t);
                // find superstate of t
                (void)QHSM_RESERVED_EVT_(t, Q_EMPTY_SIG);
            }
            t = me->temp.fun;
        }

        // take the tran...
        ip = QHsm_tran_simple_(me, &path[0], qsId);
        if (ip < -1) { // not a simple tran.?
            ip = QHsm_tran_complex_(me, &path[0], qsId);
        }

        QHsm_enter_target_(me, &path[0], ip, qsId);
        t = path[0];
        QS_TRAN_END_(QS_QEP_TRAN, s, t);
    }
#ifdef Q_SPY
    else if (r == Q_RET_HANDLED) {
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s);
    }
    else {
        QS_TRAN0_(QS_QEP_IGNORED, me->state.fun);
    }
#endif // Q_SPY

    me->state.fun = t; // change the current active state
}

//............................................................................
//! @private @memberof QHsm
bool QHsm_isIn_(
    QAsm * const me,
    QStateHandler const stateHndl)
{
    bool inState = false; // assume that this HSM is not in 'state'

    // scan the state hierarchy bottom-up
    QStateHandler s = me->state.fun;
    QState r = Q_RET_SUPER;
    while (r != Q_RET_IGNORED) {
        if (s == stateHndl) { // do the states match?
            inState = true;  // 'true' means that match found
            break; // break out of the for-loop
        }
        r = QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG);
        s = me->temp.fun;
    }
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
//! @protected @memberof QHsm
QState QHsm_top(QHsm const * const me,
    QEvt const * const e)
{
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(e);
    return Q_RET_IGNORED; // the top state ignores all events
}

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
    while (r != Q_RET_IGNORED) {
        // have the parent of the current child?
        if (me->super.temp.fun == parentHndl) {
#ifndef Q_UNSAFE
            isFound = true; // indicate that child state was found
#endif
            break;
        }
        child = me->super.temp.fun;
        r = QHSM_RESERVED_EVT_(child, Q_EMPTY_SIG);
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(590, isFound);
    QF_CRIT_EXIT();

    return child;
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

    QStateHandler t = path[0];
    QStateHandler const s = path[2];
    int_fast8_t ip = 0; // tran. entry path index
    QS_CRIT_STAT

    // (a) check source==target (tran. to self)...
    if (s == t) {
        // exit source s
        if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
        }
        ip = 0; // enter the target
    }
    else {
        // find superstate of target
        (void)QHSM_RESERVED_EVT_(t, Q_EMPTY_SIG);

        t = me->temp.fun;

        // (b) check source==target->super...
        if (s == t) {
            ip = 0; // enter the target
        }
        else {
            // find superstate of src
            (void)QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG);

            // (c) check source->super==target->super...
            if (me->temp.fun == t) {
                // exit source s
                if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                }
                ip = 0; // enter the target
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
                ip = -2; // cause execution of complex tran.
            }
        }
    }
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

    // (e) check rest of source==target->super->super..
    // and store the entry path along the way
    int_fast8_t iq = 0; // indicate that LCA was found
    int_fast8_t ip = 1; // enter target and its superstate
    QStateHandler const s = path[2]; // source state
    QStateHandler t = me->temp.fun;  // source->super
    QF_CRIT_STAT

    // find target->super->super...
    // note: ip is the fixed upper loop bound
    QState r = QHSM_RESERVED_EVT_(path[1], Q_EMPTY_SIG);
    while ((r == Q_RET_SUPER) && (ip < (QHSM_MAX_NEST_DEPTH_ - 1))) {
        ++ip;
        path[ip] = me->temp.fun; // store the entry path
        if (me->temp.fun == s) { // is it the source?
            iq = 1; // indicate that the LCA found
            --ip;   // do not enter the source
            break;  // terminate the loop
        }
        r = QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);
    }
    QF_CRIT_ENTRY();
    Q_INVARIANT_INCRIT(711, ip < (QHSM_MAX_NEST_DEPTH_ - 1));
    QF_CRIT_EXIT();

    // the LCA not found yet?
    if (iq == 0) {

        // exit source s
#ifndef Q_SPY
        (void)QHSM_RESERVED_EVT_(s, Q_EXIT_SIG);
#else
        if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
        }
#endif // def Q_SPY

        // (f) check the rest of
        // source->super == target->super->super...
        iq = ip;
        r = Q_RET_IGNORED; // indicate that the LCA NOT found
        // note: iq is the fixed upper loop bound
        do {
            if (t == path[iq]) { // is this the LCA?
                r = Q_RET_HANDLED; // indicate the LCA found
                ip = iq - 1; // do not enter the LCA
                break; // terminate the loop
            }
            --iq; // try lower superstate of target
        } while (iq >= 0);

        if (r != Q_RET_HANDLED) { // the LCA still not found?
            // (g) check each source->super->...
            // for each target->super...
            r = Q_RET_SUPER; // keep looping
            while (r != Q_RET_HANDLED) {
                // exit from t
                if (QHSM_RESERVED_EVT_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, t);
                    // find superstate of t
                    (void)QHSM_RESERVED_EVT_(t, Q_EMPTY_SIG);
                }
                t = me->temp.fun; // set to super of t
                iq = ip;
                do {
                    if (t == path[iq]) { // is this the LCA?
                        ip = iq - 1; // do not enter the LCA
                        r = Q_RET_HANDLED; // break outer loop
                        break;  // terminate the inner loop
                    }
                    --iq; // try lower superstate of target
                } while (iq >= 0);
            }
        }
    }
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

    QF_CRIT_STAT

    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(800, depth < QHSM_MAX_NEST_DEPTH_);
    QF_CRIT_EXIT();

    int_fast8_t ip = depth;
    // execute state entry actions in the desired order...
    // NOTE: ip is the fixed upper loop bound
    for (; ip >= 0; --ip) {
        // enter path[ip]
        if (QHSM_RESERVED_EVT_(path[ip], Q_ENTRY_SIG)
            == Q_RET_HANDLED)
        {
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);
        }
    }
    QStateHandler t = path[0];
    me->temp.fun = t; // update the next state

    // drill into the target hierarchy...
    while (QHSM_RESERVED_EVT_(t, Q_INIT_SIG) == Q_RET_TRAN) {

        QS_TRAN_SEG_(QS_QEP_STATE_INIT, t, me->temp.fun);

        path[0] = me->temp.fun;

        // find superstate
        (void)QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);

        ip = 0; // note: ip is the fixed upper loop bound
        while ((me->temp.fun != t) && (ip < (QHSM_MAX_NEST_DEPTH_ - 1))) {
            ++ip;
            path[ip] = me->temp.fun;
            // find superstate
            (void)QHSM_RESERVED_EVT_(me->temp.fun, Q_EMPTY_SIG);
        }
        QF_CRIT_ENTRY();
        // too many state nesting levels or "malformed" HSM
        Q_INVARIANT_INCRIT(891, ip < QHSM_MAX_NEST_DEPTH_);
        QF_CRIT_EXIT();

        me->temp.fun = path[0];

        // retrace the entry path in reverse (correct) order...
        // note: ip is the fixed upper loop bound
        do {
            // enter path[ip]
            if (QHSM_RESERVED_EVT_(path[ip], Q_ENTRY_SIG) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);
            }
            --ip;
        } while (ip >= 0);

        t = path[0]; // current state becomes the new source
    }
}
