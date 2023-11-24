/* TimeBomb/Button with QP/C framework */
#include "qpc.h"  /* QP/C API */
#include "bsp.h"

Q_DEFINE_THIS_MODULE("main") /* this module name for Q_ASSERT() */

/* The TimeBomb AO =======================================================*/
typedef struct TimeBomb TimeBomb;
struct TimeBomb {
    QActive super; /* inherit QActive base class */
    /* add private data for the AO... */
    QTimeEvt te;
    uint32_t blink_ctr;
};

QState TimeBomb_initial(TimeBomb * const me, QEvt const * const e);
QState TimeBomb_armed(TimeBomb * const me, QEvt const * const e);
QState TimeBomb_wait4button(TimeBomb * const me, QEvt const * const e);
QState TimeBomb_blink(TimeBomb * const me, QEvt const * const e);
QState TimeBomb_pause(TimeBomb * const me, QEvt const * const e);
QState TimeBomb_boom(TimeBomb * const me, QEvt const * const e);
QState TimeBomb_defused(TimeBomb * const me, QEvt const * const e);


QState TimeBomb_initial(TimeBomb * const me, QEvt const * const e) {
    return Q_TRAN(TimeBomb_wait4button);
}

QState TimeBomb_armed(TimeBomb * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_EXIT_SIG: {
            BSP_ledRedOff();
            BSP_ledGreenOff();
            BSP_ledBlueOff();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(TimeBomb_wait4button);
            break;
        }
        case BUTTON2_PRESSED_SIG: {
            status = Q_TRAN(TimeBomb_defused);
            break;
        }
        default: {
            status = Q_SUPER(QHsm_top);
            break;
        }
    }
    return status;
}


QState TimeBomb_wait4button(TimeBomb * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledGreenOn();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            BSP_ledGreenOff();
            status = Q_HANDLED();
            break;
        }
        case BUTTON_PRESSED_SIG: {
            me->blink_ctr = 5U;
            status = Q_TRAN(TimeBomb_blink);
            break;
        }
        default: {
            status = Q_SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

QState TimeBomb_blink(TimeBomb * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledRedOn();
            QTimeEvt_armX(&me->te, BSP_TICKS_PER_SEC/2, 0U);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            BSP_ledRedOff();
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            status = Q_TRAN(TimeBomb_pause);
            break;
        }
        default: {
            status = Q_SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

QState TimeBomb_pause(TimeBomb * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            QTimeEvt_armX(&me->te, BSP_TICKS_PER_SEC/2, 0U);
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            --me->blink_ctr;
            if (me->blink_ctr > 0U) {
                status = Q_TRAN(TimeBomb_blink);
            }
            else {
                status = Q_TRAN(TimeBomb_boom);
            }
            break;
        }
        default: {
            status = Q_SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

QState TimeBomb_boom(TimeBomb * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledRedOn();
            BSP_ledGreenOn();
            BSP_ledBlueOn();
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(TimeBomb_armed);
            break;
        }
    }
    return status;
}

QState TimeBomb_defused(TimeBomb * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledBlueOn();
            status = Q_HANDLED();
            break;
        }
        case BUTTON2_PRESSED_SIG: {
            status = Q_TRAN(TimeBomb_armed);
            break;
        }
        default: {
            status = Q_SUPER(QHsm_top);
            break;
        }
    }
    return status;
}


void TimeBomb_ctor(TimeBomb * const me) {
    QActive_ctor(&me->super, (QStateHandler)&TimeBomb_initial);
    QTimeEvt_ctorX(&me->te, &me->super, TIMEOUT_SIG, 0U);
}

static QEvt const *timeBomb_queue[10];
static TimeBomb timeBomb;
QActive *AO_TimeBomb = &timeBomb.super;

/* the main function =========================================================*/
int main() {

    BSP_init(); /* initialize the BSP */
    QF_init();   /* initialize QP/C */

    /* create AO and start it */
    TimeBomb_ctor(&timeBomb);
    QACTIVE_START(AO_TimeBomb,
                 2U,
                 timeBomb_queue,
                 sizeof(timeBomb_queue)/sizeof(timeBomb_queue[0]),
                 (void *)0, 0U,
                 (void *)0);

    QF_run(); /* run QP/C... */
    return 0; /* NOTE: the scheduler does NOT return */
}
