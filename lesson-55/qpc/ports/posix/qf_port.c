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

// expose features from the 2008 POSIX standard (IEEE Standard 1003.1-2008)
#define _POSIX_C_SOURCE 200809L

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

#include <limits.h>       // for PTHREAD_STACK_MIN
#include <sys/mman.h>     // for mlockall()
#include <sys/ioctl.h>
#include <time.h>         // for clock_nanosleep()
#include <string.h>       // for memcpy() and memset()
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects =============================================================
// initialize the startup mutex with default non-recursive initializer
static pthread_mutex_t l_startupMutex = PTHREAD_MUTEX_INITIALIZER;

static bool l_isRunning;       // flag indicating when QF is running
static struct timespec l_tick; // structure for the clock tick
static int_t l_tickPrio;       // priority of the ticker thread

#define NSEC_PER_SEC           1000000000L
#define DEFAULT_TICKS_PER_SEC  100L

static void sigIntHandler(int dummy); // prototype
static void sigIntHandler(int dummy) {
    Q_UNUSED_PAR(dummy);
    QF_onCleanup();
    exit(-1);
}

//----------------------------------------------------------------------------
#ifdef __APPLE__

#define TIMER_ABSTIME 0

// emulate clock_nanosleep() for CLOCK_MONOTONIC and TIMER_ABSTIME
static inline int clock_nanosleep(clockid_t clockid, int flags,
    const struct timespec* t,
    struct timespec* remain)
{
    Q_UNUSED_PAR(clockid);
    Q_UNUSED_PAR(flags);
    Q_UNUSED_PAR(remain);

    struct timespec ts_delta;
    clock_gettime(CLOCK_MONOTONIC, &ts_delta);

    ts_delta.tv_sec  = t->tv_sec  - ts_delta.tv_sec;
    ts_delta.tv_nsec = t->tv_nsec - ts_delta.tv_nsec;
    if (ts_delta.tv_sec < 0) {
        ts_delta.tv_sec = 0;
        ts_delta.tv_nsec = 0;
    }
    else if (ts_delta.tv_nsec < 0) {
        if (ts_delta.tv_sec == 0) {
            ts_delta.tv_sec = 0;
            ts_delta.tv_nsec = 0;
        }
        else {
            ts_delta.tv_sec = ts_delta.tv_sec - 1;
            ts_delta.tv_nsec = ts_delta.tv_nsec + NSEC_PER_SEC;
        }
    }

    return nanosleep(&ts_delta, NULL);
}
#endif

//============================================================================
// QF functions

// NOTE: initialize the critical section mutex as non-recursive,
// but check that nesting of critical sections never occurs
// (see QF_enterCriticalSection_()/QF_leaveCriticalSection_()
pthread_mutex_t QF_critSectMutex_ = PTHREAD_MUTEX_INITIALIZER;
int_t QF_critSectNest_;

//............................................................................
void QF_enterCriticalSection_(void) {
    pthread_mutex_lock(&QF_critSectMutex_);
    Q_ASSERT_INCRIT(100, QF_critSectNest_ == 0); // NO nesting of crit.sect!
    ++QF_critSectNest_;
}
//............................................................................
void QF_leaveCriticalSection_(void) {
    Q_ASSERT_INCRIT(200, QF_critSectNest_ == 1); // crit.sect. must balance!
    if ((--QF_critSectNest_) == 0) {
        pthread_mutex_unlock(&QF_critSectMutex_);
    }
}

//............................................................................
void QF_init(void) {
    // lock memory so we're never swapped out to disk
    //mlockall(MCL_CURRENT | MCL_FUTURE); // un-comment when supported

    // lock the startup mutex to block any active objects started before
    // calling QF_run()
    pthread_mutex_lock(&l_startupMutex);

    QTimeEvt_init(); // initialize QTimeEvts

    l_tick.tv_sec = 0;
    l_tick.tv_nsec = NSEC_PER_SEC / DEFAULT_TICKS_PER_SEC; // default rate
    l_tickPrio = sched_get_priority_min(SCHED_FIFO); // default ticker prio

    // install the SIGINT (Ctrl-C) signal handler
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = &sigIntHandler;
    sigaction(SIGINT, &sig_act, NULL);
}

//............................................................................
int QF_run(void) {

    QF_onStartup(); // application-specific startup callback

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    // try to set the priority of the ticker thread, see NOTE01
    struct sched_param sparam;
    sparam.sched_priority = l_tickPrio;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam) == 0) {
        // success, this application has sufficient privileges
    }
    else {
        // setting priority failed, probably due to insufficient privileges
    }

    // exit the startup critical section to unblock any active objects
    // started before calling QF_run()
    pthread_mutex_unlock(&l_startupMutex);
    l_isRunning = true;

    // The provided clock tick service configured?
    if ((l_tick.tv_sec != 0) || (l_tick.tv_nsec != 0)) {

        // get the absolute monotonic time for no-drift sleeping
        static struct timespec next_tick;
        clock_gettime(CLOCK_MONOTONIC, &next_tick);

        // round down nanoseconds to the nearest configured period
        next_tick.tv_nsec
            = (next_tick.tv_nsec / l_tick.tv_nsec) * l_tick.tv_nsec;

        while (l_isRunning) { // the clock tick loop...

            // advance to the next tick (absolute time)
            next_tick.tv_nsec += l_tick.tv_nsec;
            if (next_tick.tv_nsec >= NSEC_PER_SEC) {
                next_tick.tv_nsec -= NSEC_PER_SEC;
                next_tick.tv_sec  += 1;
            }

            // sleep without drifting till next_time (absolute), see NOTE03
            if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                                &next_tick, NULL) == 0) // success?
            {
                // clock tick callback (must call QTIMEEVT_TICK_X() once)
                QF_onClockTick();
            }
        }
    }
    else { // The provided system clock tick NOT configured

        while (l_isRunning) { // the clock tick loop...

            // In case the application intentionally DISABLED the provided
            // system clock, the QF_onClockTick() callback is used to let
            // the application implement the alternative tick service.
            // In that case the QF_onClockTick() must internally WAIT
            // for the desired clock period before calling QTIMEEVT_TICK_X().
            QF_onClockTick();
        }
    }
    QF_onCleanup(); // application cleanup callback
    QS_EXIT();      // cleanup the QSPY connection

    pthread_mutex_destroy(&l_startupMutex);
    pthread_mutex_destroy(&QF_critSectMutex_);

    return 0; // return success
}
//............................................................................
void QF_stop(void) {
    l_isRunning = false; // terminate the main (ticker) thread
}
//............................................................................
void QF_setTickRate(uint32_t ticksPerSec, int tickPrio) {
    if (ticksPerSec != 0U) {
        l_tick.tv_nsec = NSEC_PER_SEC / ticksPerSec;
    }
    else {
        l_tick.tv_nsec = 0U; // means NO system clock tick
    }
    l_tickPrio = tickPrio;
}

// console access ============================================================
#ifdef QF_CONSOLE

#include <termios.h>

static struct termios l_tsav;  // structure with saved terminal attributes

void QF_consoleSetup(void) {
    struct termios tio;   // modified terminal attributes

    tcgetattr(0, &l_tsav); // save the current terminal attributes
    tcgetattr(0, &tio);    // obtain the current terminal attributes
    tio.c_lflag &= ~(ICANON | ECHO); // disable the canonical mode & echo
    tcsetattr(0, TCSANOW, &tio);     // set the new attributes
}
//............................................................................
void QF_consoleCleanup(void) {
    tcsetattr(0, TCSANOW, &l_tsav); // restore the saved attributes
}
//............................................................................
int QF_consoleGetKey(void) {
    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);
    if (byteswaiting > 0) {
        char ch;
        byteswaiting = read(0, &ch, 1);
        return (int)ch;
    }
    return 0; // no input at this time
}
//............................................................................
int QF_consoleWaitForKey(void) {
    return (int)getchar();
}
#endif

//============================================================================
static void *thread_routine(void *arg) { // the expected POSIX signature
    QActive *act = (QActive *)arg;

    // block this thread until the startup mutex is unlocked from QF_run()
    pthread_mutex_lock(&l_startupMutex);
    pthread_mutex_unlock(&l_startupMutex);

#ifdef QACTIVE_CAN_STOP
    act->thread = true;
    while (act->thread)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = QActive_get_(act); // wait for event
        QASM_DISPATCH(&act->super, e, act->prio); // dispatch to the HSM
        QF_gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    QActive_unregister_(act); // un-register this active object
#endif
    return (void *)0; // return success
}

// QActive functions =======================================================
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, uint_fast16_t const qLen,
    void * const stkSto, uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // p-threads allocate stack internally
    Q_REQUIRE_ID(800, stkSto == (void *)0);

    QEQueue_init(&me->eQueue, qSto, qLen);
    pthread_cond_init(&me->osObject, NULL);

    me->prio  = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = 0U; // preemption-threshold (not used in this port)
    QActive_register_(me); // register this AO

    // top-most initial tran. (virtual call)
    (*me->super.vptr->init)(&me->super, par, me->prio);
    QS_FLUSH(); // flush the trace buffer to the host

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // SCHED_FIFO corresponds to real-time preemptive priority-based scheduler
    // NOTE: This scheduling policy requires the superuser privileges
    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

    // priority of the p-thread, see NOTE04
    struct sched_param param;
    param.sched_priority = me->prio
                           + (sched_get_priority_max(SCHED_FIFO)
                              - QF_MAX_ACTIVE - 3U);
    pthread_attr_setschedparam(&attr, &param);

    pthread_attr_setstacksize(&attr,
        (stkSize < (int_fast16_t)PTHREAD_STACK_MIN
        ? PTHREAD_STACK_MIN
        : stkSize));
    pthread_t thread;
    int err = pthread_create(&thread, &attr, &thread_routine, me);
    if (err != 0) {
        // Creating p-thread with the SCHED_FIFO policy failed. Most likely
        // this application has no superuser privileges, so we just fall
        // back to the default SCHED_OTHER policy and priority 0.
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        param.sched_priority = 0;
        pthread_attr_setschedparam(&attr, &param);
        err = pthread_create(&thread, &attr, &thread_routine, me);
    }
    Q_ASSERT_ID(810, err == 0); // AO thread must be created

    //pthread_attr_getschedparam(&attr, &param);
    //printf("param.sched_priority==%d\n", param.sched_priority);

    pthread_attr_destroy(&attr);
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive_stop(QActive * const me) {
    if (QActive_subscrList_ != (QSubscrList *)0) {
        QActive_unsubscribeAll(me); // unsubscribe from all events
    }
    me->thread = false; // stop the thread loop (see thread_routine())
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
// NOTE01:
// In Linux, the scheduler policy closest to real-time is the SCHED_FIFO
// policy, available only with superuser privileges. QF_run() attempts to set
// this policy as well as to maximize its priority, so that the ticking
// occurs in the most timely manner (as close to an interrupt as possible).
// However, setting the SCHED_FIFO policy might fail, most probably due to
// insufficient privileges.
//
// NOTE03:
// Any blocking system call, such as clock_nanosleep() system call can
// be interrupted by a signal, such as ^C from the keyboard. In this case this
// QF port breaks out of the event-loop and returns to main() that exits and
// terminates all spawned p-threads.
//
// NOTE04:
// According to the man pages (for pthread_attr_setschedpolicy) the only value
// supported in the Linux p-threads implementation is PTHREAD_SCOPE_SYSTEM,
// meaning that the threads contend for CPU time with all processes running on
// the machine. In particular, thread priorities are interpreted relative to
// the priorities of all other processes on the machine.
//
// This is good, because it seems that if we set the priorities high enough,
// no other process (or thread running within) can gain control over the CPU.
//
// However, QF limits the number of priority levels to QF_MAX_ACTIVE.
// Assuming that a QF application will be real-time, this port reserves the
// three highest p-thread priorities for the ISR-like threads (e.g., I/O),
// and the remaining highest-priorities for the active objects.
//

