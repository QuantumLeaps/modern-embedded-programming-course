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
static CRITICAL_SECTION l_startupCritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static int   l_tickPrio = 50;  // default priority of the "ticker" thread
static bool  l_isRunning;      // flag indicating when QF is running

static DWORD WINAPI ao_thread(LPVOID arg);

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

    QTimeEvt_init(); // initialize QTimeEvts

    // initialize and enter the startup critical section object to block
    // any active objects started before calling QF_run()
    InitializeCriticalSection(&l_startupCritSect);
    EnterCriticalSection(&l_startupCritSect);
}

//............................................................................
int QF_run(void) {

    QF_onStartup(); // application-specific startup callback

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    // leave the startup critical section to unblock any active objects
    // started before calling QF_run()
    LeaveCriticalSection(&l_startupCritSect);

    l_isRunning = true; // QF is running

    // set the ticker (this thread) priority according to selection made in
    // QF_setTickRate()
    int threadPrio = THREAD_PRIORITY_NORMAL;
    if (l_tickPrio < 33) {
        threadPrio = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if (l_tickPrio > 66) {
        threadPrio = THREAD_PRIORITY_ABOVE_NORMAL;
    }
    SetThreadPriority(GetCurrentThread(), threadPrio);

    while (l_isRunning) {
        Sleep(l_tickMsec); // wait for the tick interval
        QF_onClockTick();  // must call QTIMEEVT_TICK_X()
    }

    QF_onCleanup(); // cleanup callback
    QS_EXIT();      // cleanup the QSPY connection

    //DeleteCriticalSection(&l_startupCritSect);
    //DeleteCriticalSection(&l_win32CritSect);
    return 0; // return success
}
//............................................................................
void QF_stop(void) {
    l_isRunning = false; // terminate the main (ticker) thread
}
//............................................................................
void QF_setTickRate(uint32_t ticksPerSec, int tickPrio) {
    Q_REQUIRE_ID(600, ticksPerSec != 0U);
    l_tickMsec = 1000UL / ticksPerSec;
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

    // no external AO-stack storage needed for this port
    Q_REQUIRE_ID(800, stkSto == (void *)0);

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority
    me->pthre = 0U; // preemption-threshold (not used in QF, but in Win32)
    QActive_register_(me); // register this AO


    // create the Win32 "event" to throttle the AO's event queue
    me->osObject = CreateEvent(NULL, FALSE, FALSE, NULL);
    QEQueue_init(&me->eQueue, qSto, qLen);

    // the top-most initial tran. (virtual)
    QASM_INIT(&me->super, par, me->prio);
    QS_FLUSH(); // flush the QS trace buffer to the host

    // create a Win32 thread for the AO;
    // The thread is created with THREAD_PRIORITY_NORMAL
    me->thread = CreateThread(
        NULL,
        (stkSize < 1024U ? 1024U : stkSize),
        &ao_thread,
        me,
        0,
        NULL);
    Q_ENSURE_ID(830, me->thread != (HANDLE)0); // must succeed

    // set the priority of the Win32 thread based on the
    // "prio-threshold" field provided in the `prioSpec` parameter
    int win32Prio;
    switch ((prioSpec >> 8U) & 0xFFU) {
        case 1U:
            win32Prio = THREAD_PRIORITY_LOWEST;
            break;
        case 2U:
            win32Prio = THREAD_PRIORITY_NORMAL;
            break;
        case 3U:
            win32Prio = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        default:
            win32Prio = THREAD_PRIORITY_NORMAL;
            break;
    }
    SetThreadPriority(me->thread, win32Prio);
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive_stop(QActive * const me) {
    QActive_unsubscribeAll(me); // unsubscribe this AO from all events
    me->thread = (void *)0; // stop the thread loop (see ao_thread())
}
#endif
//............................................................................
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(attr1);
    Q_UNUSED_PAR(attr2);
    Q_ERROR_INCRIT(900); // should not be called in this QP port
}

//============================================================================
static DWORD WINAPI ao_thread(LPVOID arg) { // for CreateThread()
    QActive *act = (QActive *)arg;

    // block this thread until the startup critical section is exited
    // from QF_run()
    EnterCriticalSection(&l_startupCritSect);
    LeaveCriticalSection(&l_startupCritSect);

#ifdef QACTIVE_CAN_STOP
    while (act->thread)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = QActive_get_(act); // wait for event
        QASM_DISPATCH(&act->super, e, act->prio); // dispatch to the SM
        QF_gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    QActive_unregister_(act); // un-register this active object
#endif
    return (DWORD)0; // return success
}

