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
#ifndef Q_SPY
    #error Q_SPY must be defined to compile qube.c
#endif // Q_SPY

#define QP_IMPL       // this is QP implementation
#include "qp_port.h"  // QP port
#include "qsafe.h"    // QP Functional Safety (FuSa) System
#include "qs_port.h"  // QS port
#include "qs_pkg.h"   // QS package-scope interface

#include "qspy.h"     // QSPY parser interface

#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>   // for exit()

Q_DEFINE_THIS_MODULE("qube")

// Global objects ==========================================================
QPSet QF_readySet_;
QPSet QF_readySet_dis_;
uint_fast8_t QF_intLock_;

//............................................................................
static QActive *l_currAO;
static char l_currAO_name[QS_DNAME_LEN_MAX];

static uint8_t const Qube = 0U;

static char l_line[QS_LINE_LEN_MAX]; // last line entered by the user
static char l_cmd[QS_DNAME_LEN_MAX]; // last command part of the line

static void parse_input(void);
static void handle_evts(void);

static char const *HELP_STR =
    "Commands:\n"
    "<Enter>      exit\n"
    "?            help\n"
    "help         help\n"
    "curr <name>  set the current AO\n"
    "SIG [params] dispatch a given event to the current AO";

// terminal colors
#define B_BLUE     "\x1b[44m"
#define F_GRAY     "\x1b[30;1m"
#define F_YELLOW   "\x1b[33m"
#define F_BYELLOW  "\x1b[33;1m"

#define COLOR_DFLT "\x1b[K\x1b[0m"
#define COLOR_APP  B_BLUE F_YELLOW

//............................................................................
void Qube_setAO(QActive* ao) {
    l_currAO = ao;
}
//............................................................................
static bool cmd_evt(char const* cmd, char const* params) {
    QSignal sig = (QSignal)QSPY_findSig(cmd, (ObjType)((uintptr_t)l_currAO));
    if (sig != 0U) {
        //PRINTF_S("Event: \"%s\"(%d) params: %s\n", cmd, (int)sig, params);

        if (l_currAO != (QActive *)0) {
            QEvt const *e = Qube_onGenEvt(sig, params);
            if (e != (QEvt *)0) {
                QACTIVE_POST(l_currAO, e, &Qube);
            }
            else {
                FPRINTF_S(stderr, "ERROR: %s\n",
                          "Event could not be generated");
            }
        }
        else {
            FPRINTF_S(stderr, "ERROR: %s\n",
                      "No current Active Object found");
        }
    }
    else {
        FPRINTF_S(stderr, "ERROR: Dictionary for Event-sig=%s not found\n",
                  cmd);
    }
    return true;
}
//............................................................................
static bool cmd_help(char const* cmd, char const* params) {
    Q_UNUSED_PAR(cmd);
    Q_UNUSED_PAR(params);

    PRINTF_S("\n%s", HELP_STR);
    return true;
}
//............................................................................
static bool cmd_curr(char const* cmd, char const* params) {
    Q_UNUSED_PAR(cmd);

    KeyType key = QSPY_findObj(params);
    if (key != KEY_NOT_FOUND) {
        l_currAO = (QActive *)((intptr_t)key);
        STRNCPY_S(l_currAO_name, sizeof(l_currAO_name), params);
    }
    else {
        FPRINTF_S(stderr,
                  "ERROR: Dictionary for Active Object '%s' not found\n",
                  params);
    }
    return true;
}
//............................................................................
static void parse_input(void) {
    static struct {
        char const *name;
        bool (*handler)(char const *cmd, char const *params);
    } const commands[] = {
        { "A..Z",    &cmd_evt    }, // not used directly
        { "?",       &cmd_help   },
        { "help",    &cmd_help   },
        { "curr",    &cmd_curr   },
    };
    char* lin = l_line; // pointer within the current line
    char* cmd = l_cmd;  // pointer within the current command
    char* params;       // pointer to the parameters part in the line

    //
    // find the beginning of parameters in the current line
    // copy over the command part into l_cmd[].
    //
    for (;
         (*lin != '\n') && (lin < &l_line[Q_DIM(l_line) - 1U])
             && (cmd < &l_cmd[Q_DIM(l_cmd)]);
         ++lin, ++cmd)
    {
        if ((*lin == ' ') || (*lin == ',')) { // separator?
            ++lin; // skip the separator
            break;
        }
        else {
            *cmd = *lin;
        }
    }
    *cmd = '\0';   // zero-terminate the command
    params = lin;  // remember the start of parameters

    // scan to the end of line and replace the terminating '\n' with '\0'
    for (; (*lin != '\n') && (lin < &l_line[Q_DIM(l_line) - 1U]); ++lin) {
    }
    *lin = '\0'; // zero-terminate

    bool ok = false;
    if (('A' <= l_cmd[0]) && (l_cmd[0] <= 'Z')) { // A..Z ?
        STRNCPY_S(cmd, (&l_cmd[Q_DIM(l_cmd)] - cmd), "_SIG");
        ok = cmd_evt(l_cmd, params);
    }
    else { // other command
        // search existing command (past cmd_evt)...
        for (unsigned i = 1U; i < Q_DIM(commands); ++i) {
            if (strcmp(commands[i].name, l_cmd) == 0) {
                ok = (*commands[i].handler)(l_cmd, params);
                break;
            }
        }
    }
    if (!ok) {
        FPRINTF_S(stderr, "ERROR: command '%s' not recognized\n", l_cmd);
        PRINTF_S("\n%s\n", HELP_STR);
    }
}
//............................................................................
static void handle_evts(void) {
    while (QPSet_notEmpty(&QF_readySet_)) {
        uint_fast8_t const p = QPSet_findMax(&QF_readySet_);
        QActive* const a = QActive_registry_[p];

        // perform the run-to-completion (RTC) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //   time must be non-empty and The "Vanialla" kernel asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QEvt const* const e = QActive_get_(a);
        QS_FLUSH();
        QASM_DISPATCH(&a->super, e, a->prio);
        QS_FLUSH();
#if (QF_MAX_EPOOL > 0U)
        QF_gc(e);
        QS_FLUSH();
#endif
        if (a->eQueue.frontEvt == (QEvt*)0) { // empty queue?
            QPSet_remove(&QF_readySet_, p);
        }
    }
}

//............................................................................
#if 0
static char const* my_strtok(char* str, char delim) {
    static char* next;
    char const* token;
    if (str != (char*)0) {
        next = str;
    }
    token = next;
    if (token != (char*)0) {
        char* s = next;
        for (;;) {
            if (*s == delim) {
                *s = '\0';
                next = s + 1;
                break;
            }
            else if (*s == '\0') {
                next = (char*)0;
                break;
            }
            else {
                ++s;
            }
        }
    }
    return token;
}
#endif

// QF functions ============================================================
void QF_init(void) {
#ifdef _WIN32
    system("color"); // to support color output
#endif

    PRINTF_S("Qube %s (c) state-machine.com\n\n%s\n\n",
             QP_VERSION_STR, HELP_STR);

    // Clear the internal QF variables, so that the framework can start
    // correctly even if the startup code fails to clear the uninitialized
    // data (as is required by the C Standard).
    //
    QF_bzero_(&QF_priv_,             sizeof(QF_priv_));
    QF_bzero_(&QF_readySet_,         sizeof(QF_readySet_));
    QF_bzero_(&QF_intLock_,          sizeof(QF_intLock_));
    QF_bzero_(&QActive_registry_[0], sizeof(QActive_registry_));

    l_currAO_name[0] = '\0';

    fputs(COLOR_APP, stdout);

    // initialize the QS software tracing...
    if (QS_INIT((void*)0) == 0U) {
        Q_ERROR();
    }
}
//............................................................................
void QF_stop(void) {
    QS_onReset();
}
//............................................................................
void QF_onCleanup(void) {
    fputs(COLOR_DFLT "\n", stdout);
    PRINTF_S("%s\n", "Qube exit");
}
//............................................................................
int_t QF_run(void) {
    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // function dictionaries for the standard API
    QS_FUN_DICTIONARY(&QActive_post_);
    QS_FUN_DICTIONARY(&QActive_postLIFO_);
    QS_OBJ_DICTIONARY(&Qube);

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    QS_CRIT_EXIT();

    handle_evts(); // handle all events posted so far

    if (l_currAO == (QActive*)0) { // current AO not set?
        // take the highest-priority registered AO
        for (uint8_t p = QF_MAX_ACTIVE; p != 0U; --p) {
            if (QActive_registry_[p] != (QActive *)0) {
                l_currAO = QActive_registry_[p];
                break;
            }
        }
    }
    // still not found?
    if (l_currAO != (QActive*)0) {
        extern Dictionary QSPY_objDict;

        char const* name = Dictionary_get(&QSPY_objDict,
            (KeyType)((uintptr_t)l_currAO), (char*)0);
        if (name != (char *)0) {
            STRNCPY_S(l_currAO_name, sizeof(l_currAO_name), name);
        }
    }
    else {
        STRNCPY_S(l_currAO_name, sizeof(l_currAO_name), "?");
        PRINTF_S("%s\n", "No current Active Object found");
    }

    fputs(COLOR_DFLT "\n", stdout);

    // event loop...
    for (;;) {
        PRINTF_S("%s>", l_currAO_name);
        if (fgets(l_line, sizeof(l_line), stdin) != (char*)0) {
            if (l_line[0] == '\n') { // <Enter>?
                break;
            }
            parse_input();

            fputs(COLOR_APP, stdout);
            handle_evts();
            fputs(COLOR_DFLT "\n", stdout);
        }
    }
    QF_onCleanup();
    return 0; // return no error
}

//--------------------------------------------------------------------------
void QActive_start(QActive* const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    uint_fast16_t const qLen,
    void* const stkSto,
    uint_fast16_t const stkSize,
    void const* const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    me->prio = (uint8_t)(prioSpec & 0xFFU); // QF-priority of the AO
    me->pthre = (uint8_t)(prioSpec >> 8U);   // preemption-threshold
    QActive_register_(me); // make QF aware of this active object

    QEQueue_init(&me->eQueue, qSto, qLen); // initialize the built-in queue

    QASM_INIT(&me->super, par, me->prio); // the top-most initial tran.
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive_stop(QActive* const me) {
    QActive_unsubscribeAll(me); // unsubscribe from all events
    QActive_unregister_(me); // un-register this active object
}
#endif // def QACTIVE_CAN_STOP

//--------------------------------------------------------------------------
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    // initialize the QS transmit and receive buffers
    static uint8_t qsBuf[4 * 1024]; // buffer for QS-TX channel
    QS_initBuf(qsBuf, sizeof(qsBuf));

    // QS configuration options for this application...
    QSpyConfig const config = {
        QP_VERSION,          // version
        0U,                  // endianness (little)
        QS_OBJ_PTR_SIZE,     // objPtrSize
        QS_FUN_PTR_SIZE,     // funPtrSize
        QS_TIME_SIZE,        // tstampSize
        Q_SIGNAL_SIZE,       // sigSize
        QF_EVENT_SIZ_SIZE,   // evtSize
        QF_EQUEUE_CTR_SIZE,  // queueCtrSize
        QF_MPOOL_CTR_SIZE,   // poolCtrSize
        QF_MPOOL_SIZ_SIZE,   // poolBlkSize
        QF_TIMEEVT_CTR_SIZE, // tevtCtrSize
        { 0U, 0U, 0U, 0U, 0U, 0U} // tstamp
    };

    QSPY_config(&config, (QSPY_CustParseFun)0); // no custom parser function

    return 1U; // success
}
//............................................................................
void QS_onCleanup(void) {
}
//............................................................................
void QSPY_onPrintLn(void) {
    PRINTF_S("%s\n", QSPY_line);
}
//............................................................................
void QS_onReset(void) {
    QF_onCleanup();
    exit(0);
}
//............................................................................
// NOTE:
// No critical section in QS_onFlush() to avoid nesting of critical sections
// in case QS_onFlush() is called from Q_onError().
void QS_onFlush(void) {
    for (;;) {
        uint16_t nBytes = 1024U;
        uint8_t const* block = QS_getBlock(&nBytes);
        if (block != (uint8_t*)0) {
            QSPY_parse(block, nBytes);
        }
        else {
            break;
        }
    }
}
//............................................................................
QSTimeCtr QS_onGetTime(void) {
    static QSTimeCtr time_ctr = 0U;
    ++time_ctr;
    return time_ctr;
}

//............................................................................
Q_NORETURN Q_onError(char const * const module, int_t const id) {
    FPRINTF_S(stderr, "ERROR in %s:%d", module, id);
    QS_ASSERTION(module, id, 0U); // report assertion to QS
    QF_onCleanup();
    QS_EXIT();
    exit(-1);
}
