/*****************************************************************************
* TimeBomb class for SMC; implementation for EK-TM4C123GXL
*****************************************************************************/
#ifndef TIMEBOMB_H
#define TIMEBOMB_H

#include "timebomb_sm.h"

/* The TimeBomb AO =======================================================*/
typedef struct TimeBomb {
    Active super; /* inherit Active base class */

    struct timebombContext _fsm; /* FSM context for SMC */

    /* attributes needed in TimeBomb */
    TimeEvent te;
    uint32_t blink_ctr;
} TimeBomb;

void TimeBomb_ctor(TimeBomb * const me);
void TimeBomb_dispatch(TimeBomb * const me, Event const * const e);

#endif /* TIMEBOMB_H */
