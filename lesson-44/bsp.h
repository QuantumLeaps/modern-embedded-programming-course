#ifndef BSP_H
#define BSP_H

/* system clock tick [Hz] */
#define BSP_TICKS_PER_SEC 1000U

void BSP_init(void);

void BSP_ledRedOn(void);
void BSP_ledRedOff(void);

void BSP_ledBlueOn(void);
void BSP_ledBlueOff(void);

void BSP_ledGreenOn(void);
void BSP_ledGreenOff(void);

enum {
    TIMEOUT_SIG = Q_USER_SIG,
    BUTTON_PRESS_SIG,
    BLINK_PATTERN_SIG,

    /* keep always last */
    MAX_SIG
};

extern QActive * const AO_Blinky1;
extern QActive * const AO_Blinky2;

#endif /* BSP_H */
