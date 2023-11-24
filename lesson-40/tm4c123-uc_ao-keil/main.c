/* TimeBomb/Button with uC/OS-II RTOS */
#include "uc_ao.h"  /* uC/AO API */
#include "bsp.h"
#include <stdbool.h>

static char const this_module[] = "main"; /* this module name for Q_ASSERT() */

/* The TimeBomb AO =======================================================*/
typedef struct TimeBomb TimeBomb;
struct TimeBomb {
    Active super; /* inherit Active base class */
    /* add private data for the AO... */
    TimeEvent te;
    uint32_t blink_ctr;
};

State TimeBomb_initial(TimeBomb * const me, Event const * const e);
State TimeBomb_armed(TimeBomb * const me, Event const * const e);
State TimeBomb_wait4button(TimeBomb * const me, Event const * const e);
State TimeBomb_blink(TimeBomb * const me, Event const * const e);
State TimeBomb_pause(TimeBomb * const me, Event const * const e);
State TimeBomb_boom(TimeBomb * const me, Event const * const e);
State TimeBomb_defused(TimeBomb * const me, Event const * const e);


State TimeBomb_initial(TimeBomb * const me, Event const * const e) {
    return TRAN(TimeBomb_wait4button);
}

State TimeBomb_armed(TimeBomb * const me, Event const * const e) {
    State status;
    switch (e->sig) {
        case EXIT_SIG: {
            BSP_ledRedOff();
            BSP_ledGreenOff();
            BSP_ledBlueOff();
            status = HANDLED_STATUS;
            break;
        }
        case INIT_SIG: {
            status = TRAN(TimeBomb_wait4button);
            break;
        }
        case BUTTON2_PRESSED_SIG: {
            status = TRAN(TimeBomb_defused);
            break;
        }
        default: {
            status = SUPER(Hsm_top);
            break;
        }
    }
    return status;
}


State TimeBomb_wait4button(TimeBomb * const me, Event const * const e) {
    State status;
    switch (e->sig) {
        case ENTRY_SIG: {
            BSP_ledGreenOn();
            status = HANDLED_STATUS;
            break;
        }
        case EXIT_SIG: {
            BSP_ledGreenOff();
            status = HANDLED_STATUS;
            break;
        }
        case BUTTON_PRESSED_SIG: {
            me->blink_ctr = 5U;
            status = TRAN(TimeBomb_blink);
            break;
        }
        default: {
            status = SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

State TimeBomb_blink(TimeBomb * const me, Event const * const e) {
    State status;
    switch (e->sig) {
        case ENTRY_SIG: {
            BSP_ledRedOn();
            TimeEvent_arm(&me->te, OS_TICKS_PER_SEC/2, 0U);
            status = HANDLED_STATUS;
            break;
        }
        case EXIT_SIG: {
            BSP_ledRedOff();
            status = HANDLED_STATUS;
            break;
        }
        case TIMEOUT_SIG: {
            status = TRAN(TimeBomb_pause);
            break;
        }
        default: {
            status = SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

State TimeBomb_pause(TimeBomb * const me, Event const * const e) {
    State status;
    switch (e->sig) {
        case ENTRY_SIG: {
            TimeEvent_arm(&me->te, OS_TICKS_PER_SEC/2, 0U);
            status = HANDLED_STATUS;
            break;
        }
        case TIMEOUT_SIG: {
            --me->blink_ctr;
            if (me->blink_ctr > 0U) {
                status = TRAN(TimeBomb_blink);
            }
            else {
                status = TRAN(TimeBomb_boom);
            }
            break;
        }
        default: {
            status = SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

State TimeBomb_boom(TimeBomb * const me, Event const * const e) {
    State status;
    switch (e->sig) {
        case ENTRY_SIG: {
            BSP_ledRedOn();
            BSP_ledGreenOn();
            BSP_ledBlueOn();
            status = HANDLED_STATUS;
            break;
        }
        default: {
            status = SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

State TimeBomb_defused(TimeBomb * const me, Event const * const e) {
    State status;
    switch (e->sig) {
        case ENTRY_SIG: {
            BSP_ledBlueOn();
            status = HANDLED_STATUS;
            break;
        }
        case BUTTON2_PRESSED_SIG: {
            status = TRAN(TimeBomb_armed);
            break;
        }
        default: {
            status = SUPER(Hsm_top);
            break;
        }
    }
    return status;
}


void TimeBomb_ctor(TimeBomb * const me) {
    Active_ctor(&me->super, (StateHandler)&TimeBomb_initial);
    TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
}

OS_STK stack_timeBomb[100]; /* task stack */
static Event *timeBomb_queue[10];
static TimeBomb timeBomb;
Active *AO_TimeBomb = &timeBomb.super;

/* the main function =========================================================*/
int main() {
    INT8U err;

    BSP_init(); /* initialize the BSP */
    OSInit();   /* initialize uC/OS-II */

    /* create AO and start it */
    TimeBomb_ctor(&timeBomb);
    Active_start(AO_TimeBomb,
                 2U,
                 timeBomb_queue,
                 sizeof(timeBomb_queue)/sizeof(timeBomb_queue[0]),
                 stack_timeBomb,
                 sizeof(stack_timeBomb),
                 0U);

    BSP_start(); /* configure and start the interrupts */

    OSStart(); /* start the uC/OS-II scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}
