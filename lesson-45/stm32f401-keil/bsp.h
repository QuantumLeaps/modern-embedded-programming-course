/*****************************************************************************
* BSP for TimeBomb example with QP/C framework
*****************************************************************************/
#ifndef BSP_H
#define BSP_H

#define BSP_TICKS_PER_SEC 100

void BSP_init(void);
void BSP_start(void);

void BSP_ledRedOn(void);
void BSP_ledRedOff(void);
void BSP_ledBlueOn(void);
void BSP_ledBlueOff(void);
void BSP_ledGreenOn(void);
void BSP_ledGreenOff(void);

/*---------------------------------------------------------------------------*/
enum EventSignals {
    BUTTON_PRESSED_SIG = Q_USER_SIG,
    BUTTON_RELEASED_SIG,
    BUTTON2_PRESSED_SIG,
    BUTTON2_RELEASED_SIG,
    TIMEOUT_SIG,
    /* ... */
    MAX_SIG
};

/* Active objects... */
extern QActive *AO_TimeBomb;

#endif /* BSP_H */
