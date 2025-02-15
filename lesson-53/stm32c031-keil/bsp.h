#ifndef __BSP_H__
#define __BSP_H__

enum {
    TASK_AIRBAG_PRIO = 5,
    TASK_ABS_PRIO = 2,
};

void BSP_init(void);
void BSP_goToSleep(void);
void BSP_deployAirbag(void);
void BSP_engageABS(void);

void BSP_intDisable(void);
void BSP_intEnable(void);

_Noreturn void assert_failed(char const * const module, int const id);

#endif // __BSP_H__
