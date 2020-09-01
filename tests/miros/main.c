#include <stdint.h>
#include "bsp.h"
#include "miros.h" /* MInimal Real-time Operating System */

OSThread thread_blinky1;
uint32_t stack_blinky1[40];
void main_blinky1(void) {
    while (1) {
        BSP_ledGreenOn();
        //BSP_delay(BSP_TICKS_PER_SEC / 4U);
        BSP_ledGreenOff();
        //BSP_delay(BSP_TICKS_PER_SEC * 3U / 4U);
    }
}

OSThread thread_blinky2;
uint32_t stack_blinky2[40];
void main_blinky2(void) {
    while (1) {
        BSP_ledBlueOn();
        //BSP_delay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledBlueOff();
        //BSP_delay(BSP_TICKS_PER_SEC / 2U);
    }
}

int main() {
    OS_init();
    BSP_init();

    OSThread_start(&thread_blinky1,
                   &main_blinky1,
                   stack_blinky1, sizeof(stack_blinky1));

    OSThread_start(&thread_blinky2,
                   &main_blinky2,
                   stack_blinky2, sizeof(stack_blinky2));

    OS_run();

    return 0; /* NOTE: OS_run() does not return */
}
