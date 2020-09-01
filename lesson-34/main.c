/* BlinkyButton/Button with uC/AO active-object framework */
#include "uc_ao.h"  /* uC/AO API */
#include "bsp.h"
#include <stdbool.h>

static char const this_module[] = "main"; /* this module name for Q_ASSERT() */

/* The BlinkyButton AO =======================================================*/
enum { INITIAL_BLINK_TIME = (OS_TICKS_PER_SEC / 4) };

typedef struct {
    Active super; /* inherit Active base class */
    /* add private data for the AO... */
    TimeEvent te;
    bool isLedOn;
    uint32_t blink_time;
} BlinkyButton;

static void BlinkyButton_dispatch(BlinkyButton * const me, Event const * const e) {
    switch (e->sig) {
        case INIT_SIG:
            BSP_ledBlueOff();
            /* intentionally fall through...*/
        case TIMEOUT_SIG: {
            if (!me->isLedOn) { /* LED not on */
                BSP_ledGreenOn();
                me->isLedOn = true;
                TimeEvent_arm(&me->te, me->blink_time, 0U);
            }
            else {  /* LED is on */
                BSP_ledGreenOff();
                me->isLedOn = false;
                TimeEvent_arm(&me->te, me->blink_time * 3U, 0U);
            }
            break;
        }
        case BUTTON_PRESSED_SIG: {
            INT8U err; /* uC/OS-II error status */

            BSP_ledBlueOn();

            me->blink_time >>= 1; /* shorten the blink time by factor of 2 */
            if (me->blink_time == 0U) {
                me->blink_time = INITIAL_BLINK_TIME;
            }
            break;
        }
        case BUTTON_RELEASED_SIG: {
            BSP_ledBlueOff();
            break;
        }
        default: {
            break;
        }
    }
}

void BlinkyButton_ctor(BlinkyButton * const me) {
    Active_ctor(&me->super, (DispatchHandler)&BlinkyButton_dispatch);
    TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
    me->isLedOn = false;
    me->blink_time = INITIAL_BLINK_TIME;
}

OS_STK stack_blinkyButton[100]; /* task stack */
static Event *blinkyButton_queue[10];
static BlinkyButton blinkyButton;
Active *AO_BlinkyButton = &blinkyButton.super;

/* the main function =========================================================*/
int main() {
    INT8U err;

    BSP_init(); /* initialize the BSP */
    OSInit();   /* initialize uC/OS-II */

    /* create AO and start it */
    BlinkyButton_ctor(&blinkyButton);
    Active_start(AO_BlinkyButton,
                 2U,
                 blinkyButton_queue,
                 sizeof(blinkyButton_queue)/sizeof(blinkyButton_queue[0]),
                 stack_blinkyButton,
                 sizeof(stack_blinkyButton),
                 0U);

    BSP_start(); /* configure and start the interrupts */

    OSStart(); /* start the uC/OS-II scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}

