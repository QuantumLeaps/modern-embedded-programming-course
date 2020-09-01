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
#include "uc_ao.h" /* uC/AO interface */

static char const this_module[] = "uc_ao"; /* this module name for Q_ASSERT() */

/*---------------------------------------------------------------------------*/
/* Hierarchical State Machine facilities... */
static Event const entryEvt = { ENTRY_SIG };
static Event const exitEvt  = { EXIT_SIG };
        
void Hsm_ctor(Hsm * const me, StateHandler initial) {
    me->state = initial;
}

void Hsm_init(Hsm * const me, Event const * const e) {
    Q_ASSERT(me->state != (StateHandler)0);
    (*me->state)(me, e);
    (*me->state)(me, &entryEvt);
}

void Hsm_dispatch(Hsm * const me, Event const * const e) {
    State status;
    StateHandler prev_state = me->state; /* save for later */

    Q_ASSERT(me->state != (StateHandler)0);
    status = (*me->state)(me, e);
    
    if (status == TRAN_STATUS) { /* transition taken? */
        (*prev_state)(me, &exitEvt);
        (*me->state)(me, &entryEvt);
    }
}

State Hsm_top(Hsm * const me, Event const * const e) {
    return IGNORED_STATUS;
}

/*..........................................................................*/
void Active_ctor(Active * const me, StateHandler initial) {
    Hsm_ctor(&me->super, initial);
}

/*..........................................................................*/
/* Thread function for all Active Objects (uC/OS-II task signature) */
static void Active_eventLoop(void *pdata) {
    Active *me = (Active *)pdata; /* the AO instance "me" */

    /* initialize the AO */
    Hsm_init(&me->super, (Event *)0);

    /* event loop ("message pump") */
    while (1) {
        Event *e; /* pointer to event object ("message") */
        INT8U err; /* uC/OS-II error status */

        /* wait for any event and receive it into object 'e' */
        e = OSQPend(me->queue, 0U, &err); /* BLOCKING! */
        Q_ASSERT(err == 0U);

        /* dispatch event to the active object 'me' */
        Hsm_dispatch(&me->super, e); /* NO BLOCKING! */
    }
}

/*..........................................................................*/
void Active_start(Active * const me,
                  uint8_t prio,       /* priority (1-based) */
                  Event **queueSto,
                  uint32_t queueLen,
                  void *stackSto,
                  uint32_t stackSize,
                  uint16_t opt)
{
    INT8U err;
    OS_STK *stk_sto = stackSto;
    INT32U stk_depth = (stackSize / sizeof(OS_STK));

    /* precondition */
    Q_ASSERT(me /* AO instance must be provided (cannot be NULL) */
             && (0 < prio) && (prio < OS_LOWEST_PRIO - 2U));

    me->queue = OSQCreate((void **)queueSto, queueLen);
    Q_ASSERT(me->queue); /* queue must be created */

    me->thread = OS_LOWEST_PRIO - 2U - prio; /* uC/OS-II priority */
    err = OSTaskCreateExt(
              &Active_eventLoop,        /* the thread function */
              me,                       /* the 'pdata' parameter */
#if OS_STK_GROWTH
              &stk_sto[stk_depth - 1],  /* ptos */
#else
              stk_sto,                  /* ptos */
#endif
              me->thread,               /* uC/OS-II priority */
              prio,                     /* task ID -- the unique AO priority */
              stk_sto,                  /* pbos */
              stk_depth,                /* stack depth */
              (void *)0,                /* pext */
              opt);                     /* task options */
    Q_ASSERT(err == 0U); /* thread must be created */
}

/*..........................................................................*/
void Active_post(Active * const me, Event const * const e) {
    OSQPost(me->queue, (void *)e);
}

/*--------------------------------------------------------------------------*/
/* Time Event services... */

static TimeEvent *l_tevt[10]; /* all TimeEvents in the application */
static uint_fast8_t l_tevtNum; /* current number of TimeEvents */

/*..........................................................................*/
void TimeEvent_ctor(TimeEvent * const me, Signal sig, Active *act) {
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr;
#endif

    /* no critical section because it is presumed that all TimeEvents
    * are created *before* multitasking has started.
    */
    me->super.sig = sig;
    me->act = act;
    me->timeout = 0U;
    me->interval = 0U;

    /* register one more TimeEvent instance */
    OS_ENTER_CRITICAL();
    Q_ASSERT(l_tevtNum < sizeof(l_tevt)/sizeof(l_tevt[0]));
    l_tevt[l_tevtNum] = me;
    ++l_tevtNum;
    OS_EXIT_CRITICAL();
}

/*..........................................................................*/
void TimeEvent_arm(TimeEvent * const me, uint32_t timeout, uint32_t interval) {
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr;
#endif
    OS_ENTER_CRITICAL();
    me->timeout = timeout;
    me->interval = interval;
    OS_EXIT_CRITICAL();
}

/*..........................................................................*/
void TimeEvent_disarm(TimeEvent * const me) {
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr;
#endif
    OS_ENTER_CRITICAL();
    me->timeout = 0U;
    OS_EXIT_CRITICAL();
}

/*..........................................................................*/
void TimeEvent_tick(void) {
    uint_fast8_t i;
    for (i = 0U; i < l_tevtNum; ++i) {
        TimeEvent * const t = l_tevt[i];
        Q_ASSERT(t); /* TimeEvent instance must be registered */
        if (t->timeout > 0U) { /* is this TimeEvent armed? */
            if (--t->timeout == 0U) { /* is it expiring now? */
                Active_post(t->act, &t->super);
                t->timeout = t->interval; /* rearm or disarm (one-shot) */
           }
        }
    }
}

