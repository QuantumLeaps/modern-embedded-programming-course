#ifndef __BSP_H__
#define __BSP_H__

enum {
    ISR_1 = (1U << 0U),
    ISR_2 = (1U << 1U),
    ISR_3 = (1U << 2U)
};

extern uint32_t volatile ready_set;

void BSP_init(void);
void BSP_goToSleep(void);
void BSP_deployAirbag(void);
void BSP_engageABS(void);

void BSP_intDisable(void);
void BSP_intEnable(void);

_Noreturn void assert_failed(char const * const module, int const id);

#endif // __BSP_H__
