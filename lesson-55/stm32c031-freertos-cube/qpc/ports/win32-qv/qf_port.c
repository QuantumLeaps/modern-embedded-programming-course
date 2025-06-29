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
    #include "qs_pkg.h"   // QS package-scope internal interface
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

#include <limits.h>       // limits of dynamic range for integers

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects =============================================================
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static int   l_tickPrio = 50;  // default priority of the "ticker" thread
static bool  l_isRunning;      // flag indicating when QF is running

//============================================================================
static DWORD WINAPI ticker_thread(LPVOID arg); // prototype
static DWORD WINAPI ticker_thread(LPVOID arg) { // for CreateThread()
    Q_UNUSED_PAR(arg);

    int threadPrio = THREAD_PRIORITY_NORMAL;

    // set the ticker thread priority according to selection made in
    // QF_setTickRate()
    if (l_tickPrio < 33) {
        threadPrio = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if (l_tickPrio > 66) {
        threadPrio = THREAD_PRIORITY_ABOVE_NORMAL;
    }

    SetThreadPriority(GetCurrentThread(), threadPrio);

    while (l_isRunning) { // the clock tick loop...
        Sleep(l_tickMsec); // wait for the tick interval
        QF_onClockTick(); // must call QTIMEEVT_TICK_X()
    }

    return 0U; // return success
}

// Global objects ============================================================
QPSet QF_readySet_;
QPSet QF_readySet_dis_;
HANDLE QF_win32Event_; // Win32 event to signal events
//============================================================================
// QF functions

static CRITICAL_SECTION l_win32CritSect;
static int_t l_critSectNest;   // critical section nesting up-down counter

//............................................................................
void QF_enterCriticalSection_(void) {
    EnterCriticalSection(&l_win32CritSect);
    Q_ASSERT_INCRIT(100, l_critSectNest == 0); // NO nesting of crit.sect!
    ++l_critSectNest;
}
//............................................................................
void QF_leaveCriticalSection_(void) {
    Q_ASSERT_INCRIT(200, l_critSectNest == 1); // crit.sect. must ballace!
    if ((--l_critSectNest) == 0) {
        LeaveCriticalSection(&l_win32CritSect);
    }
}

//............................................................................
void QF_init(void) {
    InitializeCriticalSection(&l_win32CritSect);
    QF_win32Event_ = CreateEvent(NULL, FALSE, FALSE, NULL);

    QPSet_setEmpty(&QF_readySet_);

    QTimeEvt_init(); // initialize QTimeEvts
}

//............................................................................
int QF_run(void) {
    l_isRunning = true; // QF is running

    QF_onStartup(); // application-specific startup callback

    QF_CRIT_STAT

    if (l_tickMsec != 0U) { // system clock tick configured?
        // create the ticker thread...
        HANDLE ticker = CreateThread(NULL, 1024, &ticker_thread,
                                    (void *)0, 0U, NULL);
#ifndef Q_UNSAFE
        QF_CRIT_ENTRY();
        Q_ASSERT_INCRIT(310, ticker != 0); // thread must be created
        QF_CRIT_EXIT();
#else
        Q_UNUSED_PAR(ticker);
#endif
    }

    // the combined event-loop and background-loop of the QV kernel
    QF_CRIT_ENTRY();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    while (l_isRunning) {
        // find the maximum priority AO ready to run
        if (QPSet_notEmpty(&QF_readySet_)) {
            uint_fast8_t p = QPSet_findMax(&QF_readySet_);
            QActive *a = QActive_registry_[p];

            // the active object 'a' must still be registered in QF
            // (e.g., it must not be stopped)
            Q_ASSERT_INCRIT(320, a != (QActive *)0);
            QF_CRIT_EXIT();

            QEvt const *e = QActive_get_(a);
            QASM_DISPATCH(&a->super, e, a->prio); // dispatch to the HSM
            QF_gc(e);

            QF_CRIT_ENTRY();
            if (a->eQueue.frontEvt == (QEvt *)0) { // empty queue?
                QPSet_remove(&QF_readySet_, p);
            }
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the Win32-QV port does not do busy-waiting
            // for events. Instead, the Win32-QV port efficiently waits until
            // QP events become available.
            QF_CRIT_EXIT();

            WaitForSingleObject(QF_win32Event_, INFINITE);

            QF_CRIT_ENTRY();
        }
    }
    QF_CRIT_EXIT();
    QF_onCleanup(); // cleanup callback
    QS_EXIT();      // cleanup the QSPY connection

    //CloseHandle(win32Event_);
    //DeleteCriticalSection(&l_win32CritSect);
    return 0; // return success
}
//............................................................................
void QF_stop(void) {
    l_isRunning = false; // terminate the main event-loop

    // unblock the event-loop so it can terminate
    QPSet_insert(&QF_readySet_, 1U);
    SetEvent(QF_win32Event_);
}
//............................................................................
void QF_setTickRate(uint32_t ticksPerSec, int tickPrio) {
    if (ticksPerSec != 0U) {
        l_tickMsec = 1000UL / ticksPerSec;
    }
    else {
        l_tickMsec = 0U; // means NO system clock tick
    }
    l_tickPrio = tickPrio;
}

// console access ============================================================
#ifdef QF_CONSOLE

#include <conio.h>        // console input/output

void QF_consoleSetup(void) {
}
//............................................................................
void QF_consoleCleanup(void) {
}
//............................................................................
int QF_consoleGetKey(void) {
    if (_kbhit()) { // any key pressed?
        return (int)_getwch();
    }
    return 0; // no input at this time
}
//............................................................................
int QF_consoleWaitForKey(void) {
    return (int)_getwch();
}

#endif // #ifdef QF_CONSOLE

// QActive functions =========================================================

void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, uint_fast16_t const qLen,
    void * const stkSto, uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // no per-AO stack needed for this port
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(800, stkSto == (void *)0);
    QF_CRIT_EXIT();

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = 0U; // preemption-threshold (not used in this port)
    QActive_register_(me); // register this AO

    QEQueue_init(&me->eQueue, qSto, qLen);

    // top-most initial tran. (virtual call)
    (*me->super.vptr->init)(&me->super, par, me->prio);
    QS_FLUSH(); // flush the QS trace buffer to the host
}

//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive_stop(QActive * const me) {
    if (QActive_subscrList_ != (QSubscrList *)0) {
        QActive_unsubscribeAll(me); // unsubscribe from all events
    }

    // make sure the AO is no longer in "ready set"
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QPSet_remove(&QF_readySet_, me->prio);
    QF_CRIT_EXIT();

    QActive_unregister_(me);
}
#endif
//............................................................................
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(attr1);
    Q_UNUSED_PAR(attr2);
    Q_ERROR_INCRIT(900); // should not be called in this QP port
}
