#include <stdint.h>
#include <intrinsics.h>
#include "tm4c_cmsis.h"
#include "bsp.h"

int main() {
    SYSCTL->GPIOHSCTL |= (1U << 5); /* enable AHB for GPIOF */
    SYSCTL->RCGC2 |= (1U << 5);  /* enable clock for GPIOF */

    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    /* turn all LEDs off */
    GPIOF_AHB->DATA_Bits[LED_RED | LED_BLUE | LED_GREEN] = 0U;

    SysTick->LOAD = SYS_CLOCK_HZ/2U - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 2) | (1U << 1) | 1U;

    __enable_interrupt();
    //GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
    while (1) {
    }
    //return 0;
}
