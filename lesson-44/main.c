#include "qpc.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE


typedef struct {
    QEvt super; /* inherit QEvt */

    /* event parameters: */
    uint8_t ticks;
    uint32_t iter;
} BlinkPatternEvt;


typedef struct {
    QActive super; /* inherit QActive */

    /* private attribures: */
    QTimeEvt te;
    uint32_t iter;
} Blinky1;

void Blinky1_ctor(Blinky1 * const me);

QState Blinky1_initial(Blinky1 * const me, void const * const par);
QState Blinky1_active(Blinky1 * const me, QEvt const * const e);

/* Blinky1 constructor */
void Blinky1_ctor(Blinky1 * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&Blinky1_initial));
    QTimeEvt_ctorX(&me->te, &me->super, TIMEOUT_SIG, 0U);
}

/* Blinky1 initial pseudostate */
QState Blinky1_initial(Blinky1 * const me, void const * const par) {
    QTimeEvt_armX(&me->te, 2U, 2U);
    me->iter = 1500U;
    return Q_TRAN(&Blinky1_active);
}
/* Blinky1 active state */
QState Blinky1_active(Blinky1 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        case BLINK_PATTERN_SIG: {
            QTimeEvt_disarm(&me->te);
            QTimeEvt_armX(&me->te, ((BlinkPatternEvt const *)e)->ticks,
                                   ((BlinkPatternEvt const *)e)->ticks);
            me->iter = ((BlinkPatternEvt const *)e)->iter;    
            status_ = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            uint32_t volatile i = me->iter;
            for (; i != 0U; --i) {
                BSP_ledGreenOn();
                BSP_ledGreenOff();
            }
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}


typedef struct {
    QActive super; /* inherit QActive */

    uint8_t seq; /* sequence counter for Blinky1 */
} Blinky2;

void Blinky2_ctor(Blinky2 * const me);

QState Blinky2_initial(Blinky2 * const me, void const * const par);
QState Blinky2_active(Blinky2 * const me, QEvt const * const e);

/* Blinky2 constructor */
void Blinky2_ctor(Blinky2 * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&Blinky2_initial));
}

/* Blinky2 initial pseudostate */
QState Blinky2_initial(Blinky2 * const me, void const * const par) {
    me->seq = 0U;
    return Q_TRAN(&Blinky2_active);
}
/* Blinky2 active state */
QState Blinky2_active(Blinky2 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        case BUTTON_PRESS_SIG: {
            enum { N_SEQ = 2 };
            static uint8_t const n_ticks[N_SEQ] = {    2U,   1U };
            static uint32_t const n_iter[N_SEQ] = { 1500U, 750U };

            /* advance and wrap-around the sequence counter */
            me->seq = (me->seq + 1U) % N_SEQ;

            BlinkPatternEvt *bpe = Q_NEW(BlinkPatternEvt,
                                         BLINK_PATTERN_SIG);
            bpe->ticks = n_ticks[me->seq];
            for (uint32_t volatile i = 1500U; i != 0U; --i) {
                BSP_ledBlueOn();
                BSP_ledBlueOff();
            }
            bpe->iter = n_iter[me->seq];
            QACTIVE_POST(AO_Blinky1, &bpe->super, 0U);

            for (uint32_t volatile i = 2*1500U; i != 0U; --i) {
                BSP_ledBlueOn();
                BSP_ledBlueOff();
            }
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

QEvt const *blinky1_queue[10]; /* queue buffer */
Blinky1 blinky1;

QEvt const *blinky2_queue[10]; /* queue buffer */
Blinky2 blinky2;

static BlinkPatternEvt evtPool[10];

QActive * const AO_Blinky1 = &blinky1.super;
QActive * const AO_Blinky2 = &blinky2.super;

int main() {
    BSP_init();
    QF_init();

    QF_poolInit(evtPool, sizeof(evtPool), sizeof(evtPool[0])); 
    
    /* initialize and start blinky1 thread */
    Blinky1_ctor(&blinky1);
    QACTIVE_START(&blinky1,
                   5U, /* priority */
                   blinky1_queue, Q_DIM(blinky1_queue), /* event queue */
                   (void *)0, 0, /* stack memory, stack size (not used) */
                   (void *)0); /* extra parameter (not used) */

    /* initialize and start blinky2 thread */
    Blinky2_ctor(&blinky2);
    QACTIVE_START(&blinky2,
                   2U, /* priority */
                   blinky2_queue, Q_DIM(blinky2_queue), /* event queue */
                   (void *)0, 0, /* stack memory, stack size (not used) */
                   (void *)0); /* extra parameter (not used) */

    /* transfer control to the RTOS to run the threads */
    return QF_run();
}
