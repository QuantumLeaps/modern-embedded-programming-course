/*****************************************************************************
* BSP for NUCLEO-C031C6 with uC/OS-II RTOS
*****************************************************************************/
#ifndef BSP_H
#define BSP_H

void BSP_init(void);
void BSP_start(void);

void BSP_ledRedOn(void);
void BSP_ledRedOff(void);
void BSP_ledBlueOn(void);
void BSP_ledBlueOff(void);
void BSP_ledGreenOn(void);
void BSP_ledGreenOff(void);

/* global RTOS objects... */
extern OS_EVENT *BSP_semaPress;   /* global semaphore handle */
extern OS_EVENT *BSP_semaRelease; /* global semaphore handle */

#endif /* BSP_H */
