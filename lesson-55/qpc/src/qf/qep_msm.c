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

#define QMSM_MAX_ENTRY_DEPTH_ ((int_fast8_t)4)

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

// internal helper macro to trace regular transition
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
#define QS_TRAN0_(rec_, trg_)           ((void)0)
#define QS_TRAN_END_(rec_, src_, trg_)  ((void)0)
#endif

//! @endcond

//============================================================================
//! @private @memberof QMsm
static QState QMsm_execTatbl_(
    QAsm * const me,
    QMTranActTable const * const tatbl,
    uint_fast8_t const qsId);

//! @private @memberof QMsm
static void QMsm_exitToTranSource_(
    QAsm * const me,
    QMState const * const cs,
    QMState const * const ts,
    uint_fast8_t const qsId);

//! @private @memberof QMsm
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
    me->super.vptr = &vtable;
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

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(200, (me->vptr != (struct QAsmVtable *)0)
        && (me->temp.fun != Q_STATE_CAST(0))
        && (me->state.obj == &l_msm_top_s));
    QF_CRIT_EXIT();

    // execute the top-most initial tran.
    QState r = (*me->temp.fun)(me, Q_EVT_CAST(QEvt));

    QF_CRIT_ENTRY();
    // the top-most initial tran. must be taken
    Q_ASSERT_INCRIT(210, r == Q_RET_TRAN_INIT);
    QF_CRIT_EXIT();

    QS_TRAN_SEG_(QS_QEP_STATE_INIT,
       me->state.obj->stateHandler, me->temp.tatbl->target->stateHandler);

    // set state to the last tran. target
    me->state.obj = me->temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    while (r >= Q_RET_TRAN_INIT) {
        // execute the tran. table
        r = QMsm_execTatbl_(me, me->temp.tatbl, qsId);
    }

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, me->state.obj->stateHandler);
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

    QMState const *s = me->state.obj; // store the current state
    QMState const *t = s;

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300,
        (e != (QEvt *)0)
        && (s != (QMState *)0));
    QF_CRIT_EXIT();

    QS_TRAN0_(QS_QEP_DISPATCH, s->stateHandler);

    // scan the state hierarchy up to the top state...
    QState r = Q_RET_SUPER;
    while (t != (QMState *)0) {
        r = (*t->stateHandler)(me, e); // call state handler function

        if (r >= Q_RET_HANDLED) { // event handled? (the most frequent case)
            break; // done scanning the state hierarchy
        }
#ifdef Q_SPY
        if (r == Q_RET_UNHANDLED) { // event unhandled due to a guard?
            QS_CRIT_ENTRY();
            QS_BEGIN_PRE(QS_QEP_UNHANDLED, qsId)
                QS_SIG_PRE(e->sig);
                QS_OBJ_PRE(me);
                QS_FUN_PRE(t->stateHandler);
            QS_END_PRE()
            QS_CRIT_EXIT();
        }
#endif
        t = t->superstate; // advance to the superstate
    }

    if (r >= Q_RET_TRAN) { // any kind of tran. taken?
        QF_CRIT_ENTRY();
        // the tran. source state must not be NULL
        Q_ASSERT_INCRIT(330, t != (QMState *)0);
        QF_CRIT_EXIT();

#ifdef Q_SPY
        QMState const * const ts = t; // tran. source for QS tracing
#endif // Q_SPY

        if (r == Q_RET_TRAN_HIST) { // was it tran. to history?
            QMState const * const hist = me->state.obj; // save history
            me->state.obj = s; // restore the original state

            QS_TRAN_SEG_(QS_QEP_TRAN_HIST,
                t->stateHandler, hist->stateHandler);

            // save the tran-action table before it gets clobbered
            struct QMTranActTable const * const tatbl = me->temp.tatbl;
            QMsm_exitToTranSource_(me, s, t, qsId);
            (void)QMsm_execTatbl_(me, tatbl, qsId);
            r = QMsm_enterHistory_(me, hist, qsId);
            s = me->state.obj;
            t = s; // set target to the current state
        }

        while (r >= Q_RET_TRAN) {
            // save the tran-action table before it gets clobbered
            struct QMTranActTable const * const tatbl = me->temp.tatbl;
            me->temp.obj = (QMState *)0; // clear
            QMsm_exitToTranSource_(me, s, t, qsId);
            r = QMsm_execTatbl_(me, tatbl, qsId);
            s = me->state.obj;
            t = s; // set target to the current state
        }

        QS_TRAN_END_(QS_QEP_TRAN, ts->stateHandler, s->stateHandler);
    }
#ifdef Q_SPY
    else if (r == Q_RET_HANDLED) { // was the event handled?
        QF_CRIT_ENTRY();
        // internal tran. source can't be NULL
        Q_ASSERT_INCRIT(380, t != (QMState *)0);
        QF_CRIT_EXIT();

        QS_TRAN0_(QS_QEP_INTERN_TRAN, t->stateHandler);
    }
    else if (t == (QMState *)0) { // event bubbled to the 'top' state?
        QS_TRAN0_(QS_QEP_IGNORED, s->stateHandler);
    }
#endif // Q_SPY
    else {
        // empty
    }
}

//............................................................................
//! @private @memberof QMsm
bool QMsm_isIn_(
    QAsm * const me,
    QStateHandler const stateHndl)
{
    bool inState = false; // assume that this SM is not in 'state'

    QMState const *s = me->state.obj;
    while (s != (QMState *)0) {
        if (s->stateHandler == stateHndl) { // match found?
            inState = true;
            break;
        }
        s = s->superstate; // advance to the superstate
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

    while (s != (QMState *)0) {
        if (s == parent) {
            isFound = true; // child is found
            break;
        }
        child = s;
        s = s->superstate;
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(590, isFound);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(isFound);
#endif

    return child; // return the child
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

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // precondition:
    // - the tran-action table pointer must not be NULL
    Q_REQUIRE_INCRIT(600, tatbl != (struct QMTranActTable *)0);
    QF_CRIT_EXIT();

    QState r = Q_RET_NULL;
    QActionHandler const *a = &tatbl->act[0];
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
    }
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(690, *a == Q_ACTION_CAST(0));
    QF_CRIT_EXIT();

    me->state.obj = (r >= Q_RET_TRAN)
        ? me->temp.tatbl->target
        : tatbl->target;
    return r;
}

//............................................................................
//! @private @memberof QMsm
static void QMsm_exitToTranSource_(
    QAsm * const me,
    QMState const * const cs,
    QMState const * const ts,
    uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif
    QS_CRIT_STAT

    // exit states from the current state to the tran. source state
    QMState const *s = cs;
    while (s != ts) {
        // exit action provided in state 's'?
        if (s->exitAction != Q_ACTION_CAST(0)) {
            // execute the exit action
            (void)(*s->exitAction)(me);

            QS_STATE_ACT_(QS_QEP_STATE_EXIT, me->temp.obj->stateHandler);
        }
        s = s->superstate; // advance to the superstate
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
    QMState const *epath[QMSM_MAX_ENTRY_DEPTH_];
    QMState const *s = hist;
    int_fast8_t i = 0; // tran. entry path index & fixed upper loop bound
    while ((s != me->state.obj) && (i < QMSM_MAX_ENTRY_DEPTH_)) {
        if (s->entryAction != Q_ACTION_CAST(0)) {
            epath[i] = s;
            ++i;
        }
        s = s->superstate;
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(810, i <= QMSM_MAX_ENTRY_DEPTH_);
    QF_CRIT_EXIT();

    // retrace the entry path in reverse (desired) order...
    // NOTE: i the fixed upper loop bound
    for (i = i - 1; i >= 0; --i) {
        // run entry action in epath[i]
        (void)(*epath[i]->entryAction)(me);

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

