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

Q_DEFINE_THIS_MODULE("qep_hsm")

// maximum depth of state nesting in a QHsm (including the top level),
// must be >= 3
#define QHSM_MAX_NEST_DEPTH_  ((size_t)6U)

//! @cond INTERNAL

// array of immutable events corresponding to the reserved signals
static QEvt const l_resEvt_[4] = {
    QEVT_INITIALIZER(Q_EMPTY_SIG),
    QEVT_INITIALIZER(Q_ENTRY_SIG),
    QEVT_INITIALIZER(Q_EXIT_SIG),
    QEVT_INITIALIZER(Q_INIT_SIG)
};

//! @endcond

//============================================================================
//! @cond INTERNAL

// internal helper macro to pass a reserved event into the state handler

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
//! @protected @memberof QHsm
void QHsm_ctor(QHsm * const me,
    QStateHandler const initial)
{
    static struct QAsmVtable const vtable = { // QAsm virtual table
        &QHsm_init_,
        &QHsm_dispatch_,
        &QHsm_isIn_,
        &QHsm_getStateHandler_
    };
    // no need to call the superclass' constructor QAsm_ctor() here
    me->super.vptr      = &vtable; // QHsm class' VTABLE
    me->super.state.fun = Q_STATE_CAST(&QHsm_top); // the current state (top)
    me->super.temp.fun  = initial; // the initial tran. handler
}

//............................................................................
//! @protected @memberof QHsm
QState QHsm_top(QHsm const * const me, QEvt const * const e) {
    // the top state handler implementation
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(e);
    return Q_RET_IGNORED; // the top state ignores all events
}

//............................................................................
//! @private @memberof QHsm
void QHsm_init_(QAsm * const me,
    void const * const e,
    uint_fast8_t const qsId)
{
#ifdef Q_SPY
    // produce QS dictionary for QHsm_top handler...
    // NOTE: the QHsm_top dictionary needs to be produced only once
    // and not every time QHsm_init_() is called.
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    bool toDo = false; // assume that dictionary not produced
    if ((QS_priv_.flags & 0x01U) == 0U) { // dictionary not produced yet?
        QS_priv_.flags |= 0x01U; // mark the QHsm_top dictionary as produced
        toDo = true;
    }
    QS_CRIT_EXIT();
    if (toDo) { // need to produce the dictionary?
        QS_FUN_DICTIONARY(&QHsm_top);
    }
#else
    Q_UNUSED_PAR(qsId);
#endif // Q_SPY

    QStateHandler const s = me->state.fun; // current state

    // current state must be initialized to QHsm_top in QHsm_ctor()
    Q_REQUIRE_LOCAL(200, s == Q_STATE_CAST(&QHsm_top));

    // temp contains the top-most initial tran. handler, which must be valid
    Q_REQUIRE_LOCAL(210, me->temp.fun != Q_STATE_CAST(0));

    // execute the top-most initial tran.
    QState const r = (*me->temp.fun)(me, Q_EVT_CAST(QEvt));

    // the top-most initial tran. must be taken
    Q_ASSERT_LOCAL(240, r == Q_RET_TRAN);

    // the top-most initial tran. must set the target in temp
    Q_ASSERT_LOCAL(250, me->temp.fun != Q_STATE_CAST(0));

    QS_TRAN_SEG_(QS_QEP_STATE_INIT, s, me->temp.fun); // output QS record

    // drill down into the state hierarchy with initial transitions...
    QStateHandler path[QHSM_MAX_NEST_DEPTH_]; // entry path array

    size_t ip = 0U; // path index & fixed loop bound
    do {
        // the entry path index must not overflow the allocated array
        Q_INVARIANT_LOCAL(260, ip < QHSM_MAX_NEST_DEPTH_);

        // the initial tran. must set target in temp
        Q_ASSERT_LOCAL(270, me->temp.fun != Q_STATE_CAST(0));

        path[ip] = me->temp.fun; // store the entry path
        ++ip;

        // find the superstate of 'm_temp.fun', ignore result
        (void)(*me->temp.fun)(me, &l_resEvt_[Q_EMPTY_SIG]);
    } while (me->temp.fun != s);

    // enter the target (possibly recursively) by initial trans.
    QHsm_enter_target_(me, path, ip, qsId);

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, path[0]); // output QS record

    me->state.fun = path[0]; // change the current active state
#ifndef Q_UNSAFE
    // establish stable state configuration
    me->temp.uint = QP_DIS_UPDATE(uintptr_t, me->state.uint);
#else
    Q_UNUSED_PAR(r);
#endif
}

//............................................................................
//! @private @memberof QHsm
void QHsm_dispatch_(QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    // this state machine must be in a "stable state configuration"
    // NOTE: stable state configuration is established after every RTC step.
    Q_INVARIANT_LOCAL(300,
        QP_DIS_VERIFY(uintptr_t, me->state.uint, me->temp.uint));

    // the event to be dispatched must be valid
    Q_REQUIRE_LOCAL(310, e != (QEvt *)0);

    QStateHandler s = me->state.fun; // current state
#ifndef Q_UNSAFE
    uintptr_t const state_dis = me->temp.uint; // from invariant-300
#endif
    me->temp.fun = Q_STATE_CAST(0); // invalidate "stable state configuration"

    QS_CRIT_STAT
    QS_TRAN0_(QS_QEP_DISPATCH, s); // output QS record

    // process the event hierarchically...
    QStateHandler path[QHSM_MAX_NEST_DEPTH_]; // entry path array
    me->temp.fun = s;
    QState r; // state handler return value
    size_t ip = QHSM_MAX_NEST_DEPTH_; // path index & fixed loop bound
    do {
        // the entry path index must stay in range of the path[] array
        Q_INVARIANT_LOCAL(340, ip != 0U);

        s = me->temp.fun; // set s to the superstate set previously
        --ip; // build the entry path[] from the end
        path[ip] = s; // store the path to potential tran. source

        r = (*s)(me, e); // try to handle event e in state s

        if (r == Q_RET_UNHANDLED) { // unhandled due to a guard?
            QS_TRAN_ACT_(QS_QEP_UNHANDLED, s); // output QS record

            // find the superstate of 's'
            r = (*s)(me, &l_resEvt_[Q_EMPTY_SIG]);
        }
    } while (r == Q_RET_SUPER); // loop as long as superstate returned

    // me->state should not change, so it must match the saved DIS
    Q_INVARIANT_LOCAL(350,
        QP_DIS_VERIFY(uintptr_t, me->state.uint, state_dis));

    if (r == Q_RET_IGNORED) { // was event ignored?
        QS_TRAN0_(QS_QEP_IGNORED, me->state.fun); // output QS record
    }
    else if (r == Q_RET_HANDLED) { // did the last handler handle the event?
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s); // output QS record
    }
    else if ((r == Q_RET_TRAN) || (r == Q_RET_TRAN_HIST)) { // tran. taken?
        // tran. must have set me->temp to the target state
        Q_ASSERT_LOCAL(360, me->temp.fun != Q_STATE_CAST(0));

#ifdef Q_SPY
        if (r == Q_RET_TRAN_HIST) { // tran. to history?
            QS_TRAN_SEG_(QS_QEP_TRAN_HIST, s, me->temp.fun);//output QS record
        }
#endif

        path[0] = me->temp.fun; // save tran. target in path[0]

        // exit current state to tran. source (path[0] not used)...
        for (size_t iq = QHSM_MAX_NEST_DEPTH_ - 1U; iq > ip; --iq) {
            // exit from 'path[iq]'
            if ((*path[iq])(me, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_EXIT, path[iq]); //output QS record
            }
        }
        path[2] = s; // save tran. source in path[2]

        // take the tran...
        ip = QHsm_tran_simple_(me, path, qsId); // try simple tran. first
        if (ip > 1U) { // not a simple tran.?
            ip = QHsm_tran_complex_(me, path, qsId);
        }

        // enter the target (possibly recursively) by initial trans.
        QHsm_enter_target_(me, path, ip, qsId);
        QS_TRAN_END_(QS_QEP_TRAN, s, path[0]); // output QS record

        me->state.fun = path[0]; // change the current active state
    }
    else {
        Q_ERROR_LOCAL(370); // last state handler returned impossible value
    }

#ifndef Q_UNSAFE
    // establish "stable state configuration"
    me->temp.uint = QP_DIS_UPDATE(uintptr_t, me->state.uint);
#endif
}

//............................................................................
//! @private @memberof QHsm
size_t QHsm_tran_simple_(
    QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QStateHandler t = path[0];       // target
    QStateHandler const s = path[2]; // source
    size_t ip = 1U; // assume to enter the target
    QS_CRIT_STAT

    // (a) check source==target (tran. to self)...
    if (s == t) {
        // exit source 's' (external tran. semantics)
        if ((*s)(me, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS record
        }
    }
    else { // not a tran. to self
        // find superstate of target in 't'
        QState const r = (*t)(me, &l_resEvt_[Q_EMPTY_SIG]);

        // state handler t must return the superstate for Q_EMPTY_SIG
        Q_ASSERT_LOCAL(440, r == Q_RET_SUPER);

        // state handler t must set temp to the superstate
        Q_ASSERT_LOCAL(450, me->temp.fun != Q_STATE_CAST(0));
#ifdef Q_UNSAFE
        Q_UNUSED_PAR(r);
#endif

        // (b) check source==target->super...
        t = me->temp.fun;
        if (s != t) {
            // find superstate of source 's', ignore the result
            (void)(*s)(me, &l_resEvt_[Q_EMPTY_SIG]);

            // (c) check source->super==target->super...
            if (me->temp.fun == t) {
                // exit source 's'
                if ((*s)(me, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS record
                }
            }
            // (d) check source->super==target...
            else if (me->temp.fun == path[0]) {
                // exit source 's'
                if ((*s)(me, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS record
                }
                ip = 0U; // set entry path index NOT to enter the target
            }
            else { // tran. more complex
                path[1] = t; // save the superstate of target
                ip = 2U; // need QHsm_tran_complex_()
            }
        }
    }
    // return: # levels in path[] for QHsm_enter_target_()
    // or indication to call QHsm_tran_complex_()
    return ip;
}

//............................................................................
//! @private @memberof QHsm
size_t QHsm_tran_complex_(QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QStateHandler s = path[2];             // tran. source
    QStateHandler const ss = me->temp.fun; // source->super
    me->temp.fun = path[1];                // target->super
    QState r;

    // (e) check rest of source == target->super->super...
    // and store the target entry path along the way
    size_t ip = 1U; // path index & fixed loop bound
    size_t iq = 0U; // assume that LCA is NOT found
    do {
        // the entry path index must stay in range of the path[] array
        Q_INVARIANT_LOCAL(540, ip < QHSM_MAX_NEST_DEPTH_);

        path[ip] = me->temp.fun; // store temp in the entry path array
        ++ip;

        // find superstate of 'm_temp.fun'
        r = (*me->temp.fun)(me, &l_resEvt_[Q_EMPTY_SIG]);
        if (me->temp.fun == s) { // is temp the LCA?
            iq = 1U; // indicate that LCA is found
            break;
        }
    } while (r == Q_RET_SUPER); // loop as long as superstate reached

    if (iq == 0U) { // the LCA not found yet?
        QS_CRIT_STAT

#ifndef Q_SPY
        // exit the source 's', ignore the result
        (void)(*s)(me, &l_resEvt_[Q_EXIT_SIG]);
#else
        // exit the source 's'
        if ((*s)(me, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS record
        }
#endif // def Q_SPY

        // (f) check the rest of
        // source->super... == target->super->super...
        s = ss; // source->super
        r = Q_RET_IGNORED; // assume that the LCA NOT found
        iq = ip;
        while (iq > 0U) {
            --iq;
            if (s == path[iq]) { // is this the LCA?
                r = Q_RET_HANDLED; // indicate the LCA found
                ip = iq; // do NOT enter the LCA
                break;
            }
        }

        if (r != Q_RET_HANDLED) { // the LCA still not found?
            // (g) check each source->super->...
            // for each target->super...
            do {
                // exit from 's'
                if ((*s)(me, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                    // find superstate of 's', ignore the result
                    (void)(*s)(me, &l_resEvt_[Q_EMPTY_SIG]);
                }
                s = me->temp.fun; // set to super of s
                // NOTE: loop bounded per invariant:540
                iq = ip;
                while (iq > 0U) {
                    --iq;
                    if (s == path[iq]) { // is this the LCA?
                        ip = iq; // do NOT to enter the LCA
                        r = Q_RET_HANDLED; // cause break from outer loop
                        break;
                    }
                }
            } while (r != Q_RET_HANDLED);
        }
    }
    // # levels in path[] for QHsm_enter_target_()
    return ip;
}

//............................................................................
//! @private @memberof QHsm
void QHsm_enter_target_(QAsm * const me,
    QStateHandler * const path,
    size_t const depth,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QS_CRIT_STAT
    size_t ip = depth;

    Q_REQUIRE_LOCAL(600, ip <= QHSM_MAX_NEST_DEPTH_);

    // execute entry actions from LCA to tran target...
    while (ip > 0U) {
        --ip;
        // enter 'path[ip]'
        if ((*path[ip])(me, &l_resEvt_[Q_ENTRY_SIG]) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]); // output QS record
        }
    }
    QStateHandler t = path[0]; // tran. target

    // drill into the target hierarchy with nested initial trans...

    // take initial tran in 't'
    while ((*t)(me, &l_resEvt_[Q_INIT_SIG]) == Q_RET_TRAN) { // tran. taken?
        // temp holds the target of initial tran. and must be valid
        Q_ASSERT_LOCAL(650, me->temp.fun != Q_STATE_CAST(0));

        QS_TRAN_SEG_(QS_QEP_STATE_INIT, t, me->temp.fun); // output QS record

        // find superstate of initial tran. target...
        ip = 0U; // entry path index and fixed loop bound
        do {
            // the entry path index must stay in range of the path[] array
            Q_INVARIANT_LOCAL(660, ip < QHSM_MAX_NEST_DEPTH_);

            path[ip] = me->temp.fun; // store the entry path
            ++ip;

            // find superstate of 'temp.fun'
            QState const r = (*me->temp.fun)(me, &l_resEvt_[Q_EMPTY_SIG]);

            // the temp superstate handler must return superstate
            Q_ASSERT_LOCAL(680, r == Q_RET_SUPER);
#ifdef Q_UNSAFE
            Q_UNUSED_PAR(r);
#endif
        } while (me->temp.fun != t); //loop as long as tran.target not reached

        // retrace the entry path in reverse (correct) order...
        while (ip > 0U) {
            --ip;
            // enter 'path[ip]'
            if ((*path[ip])(me, &l_resEvt_[Q_ENTRY_SIG]) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);//output QS record
            }
        }
        t = path[0]; // tran. target becomes the new source
    }
}

//............................................................................
//! @private @memberof QHsm
bool QHsm_isIn_(QAsm * const me,
    QStateHandler const stateHndl)
{
    // this state machine must be in a stable state configuration
    // NOTE: stable state configuration is established after every RTC step.
    Q_INVARIANT_LOCAL(700,
        QP_DIS_VERIFY(uintptr_t, me->state.uint, me->temp.uint));

    bool inState = false; // assume that this HSM is NOT in 'stateHndl'

    // scan the state hierarchy bottom-up
    QStateHandler s = me->state.fun;
    QState r;
    do {
        if (s == stateHndl) { // do the states match?
            inState = true;  // 'true' means that match found
            break; // break out of the do-loop
        }

        // find superstate of 's'
        r = (*s)(me, &l_resEvt_[Q_EMPTY_SIG]);
        s = me->temp.fun;
    } while (r == Q_RET_SUPER);

#ifndef Q_UNSAFE
    // restore the invariant (stable state configuration)
    me->temp.uint = QP_DIS_UPDATE(uintptr_t, me->state.uint);
#endif
    return inState; // return the status
}

//............................................................................
//! @public @memberof QHsm
QStateHandler QHsm_childState(QHsm * const me,
    QStateHandler const parentHndl)
{
    // NOTE: this function is designed to be called during an RTC step,
    // so it does NOT assume to be called in a stable state configuration
    // and also does NOT establish stable state configuration upon exit.

#ifndef Q_UNSAFE
    bool isFound = false; // assume the child state NOT found
#endif

    QStateHandler child = me->super.state.fun; // child state to find
    me->super.temp.fun = child; // start from the current state
    QState r = Q_RET_SUPER;
    do {
        // have the parent of the current child?
        if (me->super.temp.fun == parentHndl) {
#ifndef Q_UNSAFE
            isFound = true; // indicate that child state was found
#endif
            break;
        }
        child = me->super.temp.fun;

        // find superstate of 'child' (placed in me->super.temp)
        r = (*child)(me, &l_resEvt_[Q_EMPTY_SIG]);
    } while (r == Q_RET_SUPER);
    Q_ENSURE_LOCAL(890, isFound);

    return child;
}

//............................................................................
//! @public @memberof QHsm
QStateHandler QHsm_state(QHsm const * const me) {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return me->super.state.fun;
}

//............................................................................
//! @private @memberof QHsm
QStateHandler QHsm_getStateHandler_(QAsm const * const me) {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return me->state.fun;
}
