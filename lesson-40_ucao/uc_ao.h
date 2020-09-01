/*****************************************************************************
* Active Object pattern implementation based on uC/OS-II (uC/AO)
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2020 Quantum Leaps, LLC. All rights reserved.
*
* SPDX-License-Identifier: APACHE-2.0
*
* This software is distributed by Quantum Leaps, LLC under the terms of
* Apache License Version 2.0, which is the same license used for uC/OS-II RTOS.
* The text of the license is available at: www.apache.org/licenses/LICENSE-2.0.
*
* Contact information:
* <www.state-machine.com>
* <info@state-machine.com>
*****************************************************************************/
#ifndef UC_AO_H
#define UC_AO_H

#include "ucos_ii.h"  /* uC/OS-II API, port and compile-time configuration */

/*---------------------------------------------------------------------------*/
/* Event facilities... */

typedef uint16_t Signal; /* event signal */

enum ReservedSignals {
    INIT_SIG, /* dispatched to AO before entering event-loop */
    ENTRY_SIG, /* for triggering the entry action in a state */
    EXIT_SIG,  /* for triggering the exit action from a state */
    USER_SIG  /* first signal available to the users */
};

/* Event base class */
typedef struct {
    Signal sig; /* event signal */
    /* event parameters added in subclasses of Event */
} Event;

/*---------------------------------------------------------------------------*/
/* Hierarchical State Machine facilities... */
typedef struct Hsm Hsm; /* forward declaration */

typedef enum { TRAN_STATUS, HANDLED_STATUS, IGNORED_STATUS,
               INIT_STATUS, SUPER_STATUS } State;

typedef State (*StateHandler)(Hsm * const me, Event const * const e);

#define TRAN(target_) (((Hsm *)me)->state = (StateHandler)(target_), TRAN_STATUS)

#define SUPER(super_) (((Hsm *)me)->temp = (StateHandler)(super_), SUPER_STATUS)

struct Hsm {
    StateHandler state; /* the "state variable" */
    StateHandler temp; /* temporary state to traverse state hierarchy */
};

void Hsm_ctor(Hsm * const me, StateHandler initial);
void Hsm_init(Hsm * const me, Event const * const e);
void Hsm_dispatch(Hsm * const me, Event const * const e);
State Hsm_top(Hsm * const me, Event const * const e);

/*---------------------------------------------------------------------------*/
/* Actvie Object facilities... */

typedef struct Active Active; /* forward declaration */

/* Active Object base class */
struct Active {
    Hsm super;        /* inherit Hsm */
    
    INT8U thread;     /* private thread (the unique uC/OS-II task priority) */
    OS_EVENT *queue;  /* private message queue */

    /* active object data added in subclasses of Active */
};

void Active_ctor(Active * const me, StateHandler initial);
void Active_start(Active * const me,
                  uint8_t prio,       /* priority (1-based) */
                  Event **queueSto,
                  uint32_t queueLen,
                  void *stackSto,
                  uint32_t stackSize,
                  uint16_t opt);
void Active_post(Active * const me, Event const * const e);

/*---------------------------------------------------------------------------*/
/* Time Event facilities... */

/* Time Event class */
typedef struct {
    Event super;       /* inherit Event */
    Active *act;       /* the AO that requested this TimeEvent */
    uint32_t timeout;  /* timeout counter; 0 means not armed */
    uint32_t interval; /* interval for periodic TimeEvent, 0 means one-shot */
} TimeEvent;

void TimeEvent_ctor(TimeEvent * const me, Signal sig, Active *act);
void TimeEvent_arm(TimeEvent * const me, uint32_t timeout, uint32_t interval);
void TimeEvent_disarm(TimeEvent * const me);

/* static (i.e., class-wide) operation */
void TimeEvent_tick(void);

/*---------------------------------------------------------------------------*/
/* Assertion facilities... */

#define Q_ASSERT(check_)                   \
    if (!(check_)) {                       \
        Q_onAssert(this_module, __LINE__); \
    } else (void)0

void Q_onAssert(char const *module, int loc);

#endif /* UC_AO_H */
