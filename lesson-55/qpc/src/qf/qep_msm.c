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

Q_DEFINE_THIS_MODULE("qep_msm")

#define QMSM_MAX_NEST_DEPTH_  ((int_fast8_t)6)
#define QMSM_MAX_TRAN_LENGTH_ ((int_fast8_t)(2*QMSM_MAX_NEST_DEPTH_))

//! @cond INTERNAL

// top-state object for QMsm-style state machines
static struct QMState const l_msm_top_s = {
    (struct QMState *)0,
    Q_STATE_CAST(0),
    Q_ACTION_CAST(0),
    Q_ACTION_CAST(0),
    Q_ACTION_CAST(0)
};

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
#define QS_TRAN0_(rec_, trg_)           ((void)0)
#define QS_TRAN_END_(rec_, src_, trg_)  ((void)0)
#endif

//! @endcond

//============================================================================
static QState QMsm_execTatbl_(
    QAsm * const me,
    QMTranActTable const * const tatbl,
    uint_fast8_t const qsId);

static void QMsm_exitToTranSource_(
    QAsm * const me,
    QMState const * const curr_state,
    QMState const * const tran_source,
    uint_fast8_t const qsId);

static QState QMsm_enterHistory_(
    QAsm * const me,
    QMState const *const hist,
    uint_fast8_t const qsId);

//............................................................................
//! @protected @memberof QMsm
void QMsm_ctor(QMsm * const me,
    QStateHandler const initial)
{
    static struct QAsmVtable const vtable = { // QAsm virtual table
        &QMsm_init_,
        &QMsm_dispatch_,
        &QMsm_isIn_
#ifdef Q_SPY
        ,&QMsm_getStateHandler_
#endif
    };
    // do not call the QAsm_ctor() here
    me->super.vptr      = &vtable;
    me->super.state.obj = &l_msm_top_s; // the current state (top)
    me->super.temp.fun  = initial;      // the initial tran. handler
}

//............................................................................
//! @private @memberof QMsm
void QMsm_init_(
    QAsm * const me,
    void const * const e,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    Q_REQUIRE_LOCAL(200, me->temp.fun != Q_STATE_CAST(0));
    Q_REQUIRE_LOCAL(210, me->state.obj == &l_msm_top_s);

    // execute the top-most initial tran.
    QState r = (*me->temp.fun)(me, Q_EVT_CAST(QEvt));

    // the top-most initial tran. must be taken
    Q_ASSERT_LOCAL(240, r == Q_RET_TRAN_INIT);
    Q_ASSERT_LOCAL(250, me->temp.tatbl != (struct QMTranActTable *)0);

    QS_CRIT_STAT
    QS_TRAN_SEG_(QS_QEP_STATE_INIT,
       me->state.obj->stateHandler, me->temp.tatbl->target->stateHandler);

    // set state to the last tran. target
    me->state.obj = me->temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    do {
        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(280, lbound >= 0);
        r = QMsm_execTatbl_(me, me->temp.tatbl, qsId);
    } while (r >= Q_RET_TRAN_INIT);

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, me->state.obj->stateHandler);

#ifndef Q_UNSAFE
    // establish stable state configuration
    me->temp.uint = (uintptr_t)~me->state.uint;
#endif
}

//............................................................................
//! @private @memberof QMsm
void QMsm_dispatch_(
    QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    Q_INVARIANT_LOCAL(300, me->state.uint == (uintptr_t)(~me->temp.uint));

    Q_REQUIRE_LOCAL(310, e != (QEvt *)0);

    QMState const *s = me->state.obj; // the current state
    QMState const * const t = s; // store the current state for later
    QS_CRIT_STAT
    QS_TRAN0_(QS_QEP_DISPATCH, s->stateHandler);

    // scan the state hierarchy up to the top state...
    QState r = Q_RET_SUPER;
    int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    do {
         --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(340, lbound >= 0);

        r = (*s->stateHandler)(me, e); // call state handler function
        if (r >= Q_RET_HANDLED) { // event handled? (the most frequent case)
            break; // done scanning the state hierarchy
        }
#ifdef Q_SPY
        if (r == Q_RET_UNHANDLED) { // event unhandled due to a guard?
            QS_CRIT_ENTRY();
            QS_BEGIN_PRE(QS_QEP_UNHANDLED, qsId)
                QS_SIG_PRE(e->sig);
                QS_OBJ_PRE(me);
                QS_FUN_PRE(s->stateHandler);
            QS_END_PRE()
            QS_CRIT_EXIT();
        }
#endif
        s = s->superstate; // advance to the superstate
    } while (s != (QMState *)0);

    if (s == (QMState *)0) { // event bubbled to the 'top' state?
#ifdef Q_SPY
        QS_TRAN0_(QS_QEP_IGNORED, t->stateHandler);
#endif // Q_SPY
    }
    else if (r >= Q_RET_TRAN) { // any kind of tran. taken?
#ifdef Q_SPY
        QMState const * const ts = s; // tran. source for QS tracing
#endif // Q_SPY

        if (r == Q_RET_TRAN) {
            struct QMTranActTable const * const tatbl = me->temp.tatbl;
            QMsm_exitToTranSource_(me, t, s, qsId);
            r = QMsm_execTatbl_(me, tatbl, qsId);
#ifdef Q_SPY
            s = me->state.obj;
#endif // Q_SPY
        }
        else if (r == Q_RET_TRAN_HIST) { // was it tran. to history?
            QMState const * const hist = me->state.obj; // save history
            me->state.obj = t; // restore the original state

            QS_TRAN_SEG_(QS_QEP_TRAN_HIST,
                s->stateHandler, hist->stateHandler);

            // save the tran-action table before it gets clobbered
            struct QMTranActTable const * const tatbl = me->temp.tatbl;
            QMsm_exitToTranSource_(me, t, s, qsId);
            (void)QMsm_execTatbl_(me, tatbl, qsId);
            r = QMsm_enterHistory_(me, hist, qsId);
#ifdef Q_SPY
            s = me->state.obj;
#endif // Q_SPY
        }
        else {
            // empty
        }

        lbound = QMSM_MAX_NEST_DEPTH_;
        while (r == Q_RET_TRAN_INIT) { // initial tran. in the target?

            r = QMsm_execTatbl_(me, me->temp.tatbl, qsId);
#ifdef Q_SPY
            s = me->state.obj;
#endif // Q_SPY

             --lbound; // fixed loop bound
            Q_INVARIANT_LOCAL(360, lbound >= 0);
        }

        QS_TRAN_END_(QS_QEP_TRAN, ts->stateHandler, s->stateHandler);
    }
#ifdef Q_SPY
    else if (r == Q_RET_HANDLED) { // was the event handled?
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s->stateHandler);
    }
#endif // Q_SPY
    else {
        // empty
    }

#ifndef Q_UNSAFE
    // establish stable state configuration
    me->temp.uint = (uintptr_t)~me->state.uint;
#endif
}

//............................................................................
//! @private @memberof QMsm
static QState QMsm_execTatbl_(
    QAsm * const me,
    QMTranActTable const * const tatbl,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    Q_REQUIRE_LOCAL(400, tatbl != (struct QMTranActTable *)0);

    QS_CRIT_STAT
    QState r = Q_RET_NULL;
    QActionHandler const *a = &tatbl->act[0];
    int_fast8_t lbound = QMSM_MAX_TRAN_LENGTH_;
    while (*a != Q_ACTION_CAST(0)) {
        r = (*(*a))(me); // call the action through the 'a' pointer
        ++a;

#ifdef Q_SPY
        if (r == Q_RET_ENTRY) {
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, me->temp.obj->stateHandler);
        }
        else if (r == Q_RET_EXIT) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, me->temp.obj->stateHandler);
        }
        else if (r == Q_RET_TRAN_INIT) {
            QS_TRAN_SEG_(QS_QEP_STATE_INIT,
                tatbl->target->stateHandler,
                me->temp.tatbl->target->stateHandler);
        }
        else {
            // empty
        }
#endif // Q_SPY

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(480, lbound >= 0);
    }

    me->state.obj = (r >= Q_RET_TRAN)
        ? me->temp.tatbl->target
        : tatbl->target;
    return r;
}

//............................................................................
//! @private @memberof QMsm
static void QMsm_exitToTranSource_(
    QAsm * const me,
    QMState const * const curr_state,
    QMState const * const tran_source,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif
    QS_CRIT_STAT

    // exit states from the current state to the tran. source state
    QMState const *s = curr_state;
    int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != tran_source) {
        if (s->exitAction != Q_ACTION_CAST(0)) { // exit action provided?
            (void)(*s->exitAction)(me); // execute the exit action
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, me->temp.obj->stateHandler);
        }
        s = s->superstate; // advance to the superstate

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(580, lbound >= 0);
    }
}

//............................................................................
//! @private @memberof QMsm
static QState QMsm_enterHistory_(
    QAsm * const me,
    QMState const *const hist,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    // record the entry path from current state to history
    QMState const *epath[QMSM_MAX_NEST_DEPTH_];
    QMState const *s = hist;
    int_fast8_t i = -1; // entry path index (one below [0])
    int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != me->state.obj) {
        if (s->entryAction != Q_ACTION_CAST(0)) {
            ++i;
            Q_INVARIANT_LOCAL(610, i < QMSM_MAX_NEST_DEPTH_);
            epath[i] = s;
        }
        s = s->superstate;

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(620, lbound >= 0);
    }

    QS_CRIT_STAT
    // retrace the entry path in reverse (desired) order...
    // NOTE: i the fixed loop bound
    for (; i >= 0; --i) {
        (void)(*epath[i]->entryAction)(me); // enter epath[i]
        QS_STATE_ACT_(QS_QEP_STATE_ENTRY, epath[i]->stateHandler);
    }

    me->state.obj = hist; // set current state to the tran. target

    // initial tran. present?
    QState r = Q_RET_NULL;
    if (hist->initAction != Q_ACTION_CAST(0)) {
        r = (*hist->initAction)(me); // execute the tran. action
        QS_TRAN_SEG_(QS_QEP_STATE_INIT,
            hist->stateHandler, me->temp.tatbl->target->stateHandler);
    }
    return r;
}

//............................................................................
//! @private @memberof QMsm
bool QMsm_isIn_(
    QAsm * const me,
    QStateHandler const stateHndl)
{
    bool inState = false; // assume that this SM is not in 'state'
    QMState const *s = me->state.obj;
    int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != (QMState *)0) {
        if (s->stateHandler == stateHndl) { // match found?
            inState = true;
            break;
        }
        s = s->superstate; // advance to the superstate

         --lbound; // fixed loop bound
         Q_INVARIANT_LOCAL(740, lbound >= 0);
    }

    return inState;
}

//............................................................................
#ifdef Q_SPY
//! @public @memberof QMsm
QStateHandler QMsm_getStateHandler_(QAsm * const me) {
    return me->state.obj->stateHandler;
}
#endif // def Q_SPY

//............................................................................
//! @public @memberof QMsm
QMState const * QMsm_childStateObj(QMsm const * const me,
    QMState const * const parent)
{
    QMState const *s = me->super.state.obj; // start with current state
    QMState const *child = s;
    bool isFound = false; // assume the child NOT found
    int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != (QMState *)0) {
        if (s == parent) {
            isFound = true; // child is found
            break;
        }
        child = s;
        s = s->superstate;

         --lbound; // fixed loop bound
         Q_INVARIANT_LOCAL(840, lbound >= 0);
    }
    Q_ENSURE_LOCAL(890, isFound);

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(isFound);
#endif

    return child;
}
