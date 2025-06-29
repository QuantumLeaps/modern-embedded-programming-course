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

static bool l_isRunning;       // flag indicating when QF is running
static struct timespec l_tick; // structure for the clock tick
static int_t l_tickPrio;       // priority of the ticker thread

#define NSEC_PER_SEC           1000000000L
#define DEFAULT_TICKS_PER_SEC  100L

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

//----------------------------------------------------------------------------
static void *ticker_thread(void *arg); // prototype
static void *ticker_thread(void *arg) { // for pthread_create()
    Q_UNUSED_PAR(arg);

    // system clock tick must be configured
    Q_REQUIRE_ID(100, l_tick.tv_nsec != 0);

    // get the absolute monotonic time for no-drift sleeping
    static struct timespec next_tick;
    clock_gettime(CLOCK_MONOTONIC, &next_tick);

    // round down nanoseconds to the nearest configured period
    next_tick.tv_nsec = (next_tick.tv_nsec / l_tick.tv_nsec) * l_tick.tv_nsec;

    while (l_isRunning) { // the clock tick loop...

        // advance to the next tick (absolute time)
        next_tick.tv_nsec += l_tick.tv_nsec;
        if (next_tick.tv_nsec >= NSEC_PER_SEC) {
            next_tick.tv_nsec -= NSEC_PER_SEC;
            next_tick.tv_sec  += 1;
        }

        // sleep without drifting till next_tick (absolute), see NOTE03
        if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                            &next_tick, NULL) == 0) // success?
        {
            // clock tick callback (must call QTIMEEVT_TICK_X())
            QF_onClockTick();
        }
    }
    return (void *)0; // return success
}
//............................................................................
static void sigIntHandler(int dummy); // prototype
static void sigIntHandler(int dummy) {
    Q_UNUSED_PAR(dummy);
    QF_onCleanup();
    exit(-1);
}

// Global objects ============================================================
QPSet QF_readySet_;
QPSet QF_readySet_dis_;
pthread_cond_t QF_condVar_; // Cond.var. to signal events

//============================================================================
// QF functions

// NOTE: initialize the critical section mutex as non-recursive,
// but check that nesting of critical sections never occurs
// (see QF_enterCriticalSection_()/QF_leaveCriticalSection_()
static pthread_mutex_t l_critSectMutex_ = PTHREAD_MUTEX_INITIALIZER;
static int_t l_critSectNest;   // critical section nesting up-down counter

//............................................................................
void QF_enterCriticalSection_(void) {
    pthread_mutex_lock(&l_critSectMutex_);
    Q_ASSERT_INCRIT(101, l_critSectNest == 0); // NO nesting of crit.sect!
    ++l_critSectNest;
}
//............................................................................
void QF_leaveCriticalSection_(void) {
    Q_ASSERT_INCRIT(102, l_critSectNest == 1); // crit.sect. must balance!
    if ((--l_critSectNest) == 0) {
        pthread_mutex_unlock(&l_critSectMutex_);
    }
}

//............................................................................
void QF_init(void) {
    // init the global condition variable with the default initializer
    pthread_cond_init(&QF_condVar_, NULL);

    QPSet_setEmpty(&QF_readySet_);

    // lock memory so we're never swapped out to disk
    //mlockall(MCL_CURRENT | MCL_FUTURE); // un-comment when supported

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
    l_isRunning = true; // QF is running

    QF_onStartup(); // application-specific startup callback

    QF_CRIT_STAT
    // system clock tick configured?
    if ((l_tick.tv_sec != 0) || (l_tick.tv_nsec != 0)) {

        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // SCHED_FIFO corresponds to real-time preemptive priority-based
        // scheduler.
        // NOTE: This scheduling policy requires the superuser privileges
        pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        struct sched_param param;
        param.sched_priority = l_tickPrio;

        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        pthread_t ticker;
        int err = pthread_create(&ticker, &attr, &ticker_thread, 0);
        if (err != 0) {
            // Creating the p-thread with the SCHED_FIFO policy failed.
            // Most probably this application has no superuser privileges,
            // so we just fall back to the default SCHED_OTHER policy
            // and priority 0.
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
            param.sched_priority = 0;
            pthread_attr_setschedparam(&attr, &param);
            err = pthread_create(&ticker, &attr, &ticker_thread, 0);
        }
        QF_CRIT_ENTRY();
        Q_ASSERT_INCRIT(310, err == 0); // ticker thread must be created
        QF_CRIT_EXIT();

        //pthread_attr_getschedparam(&attr, &param);
        //printf("param.sched_priority==%d\n", param.sched_priority);

        pthread_attr_destroy(&attr);
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
            // callback. However, the POSIX-QV port does not do busy-waiting
            // for events. Instead, the POSIX-QV port efficiently waits until
            // QP events become available.
            while (QPSet_isEmpty(&QF_readySet_)) {
                Q_ASSERT_INCRIT(390, l_critSectNest == 1);
                --l_critSectNest;

                pthread_cond_wait(&QF_condVar_, &l_critSectMutex_);

                Q_ASSERT_INCRIT(391, l_critSectNest == 0);
                ++l_critSectNest;
            }
        }
    }
    QF_CRIT_EXIT();
    QF_onCleanup(); // cleanup callback
    QS_EXIT();      // cleanup the QSPY connection

    pthread_cond_destroy(&QF_condVar_); // cleanup the condition variable
    pthread_mutex_destroy(&l_critSectMutex_); // cleanup the global mutex

    return 0; // return success
}
//............................................................................
void QF_stop(void) {
    l_isRunning = false; // terminate the main event-loop

    // unblock the event-loop so it can terminate
    QPSet_insert(&QF_readySet_, 1U);
    pthread_cond_signal(&QF_condVar_);
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
    Q_REQUIRE_INCRIT(600, stkSto == (void *)0);
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
    QActive_unsubscribeAll(me);

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

