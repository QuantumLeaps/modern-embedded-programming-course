#include "TM4C123GH6PM.h"
#include "bsp.h"

int main(void) {
    SYSCTL->RCGCGPIO  |= (1U << 5U); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5U); /* enable AHB for GPIOF */

#if (__ARM_FP != 0) /* if VFP available... */
    /* make sure that the FPU is enabled by seting CP10 & CP11 Full Access */
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));
#endif

    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    SysTick->LOAD = SYS_CLOCK_HZ/2U - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 2U) | (1U << 1U) | 1U;

    SysTick_Handler();

    __enable_irq();
    while (1) {
        GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
        GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;

#if (__ARM_FP != 0) /* if VFP available... */
        /* exercise the single-precision FPU */
        float x = 1.234f;
        float volatile y = x*x;
#endif
    }
    //return 0; // unreachable code
}
