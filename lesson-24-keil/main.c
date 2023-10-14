#include <stdint.h>
#include "miros.h"
#include "bsp.h"

uint32_t stack_blinky1[40];
OSThread blinky1;
void main_blinky1() {
    while (1) {
        BSP_ledGreenOn();
        //BSP_delay(BSP_TICKS_PER_SEC / 4U);
        BSP_ledGreenOff();
        //BSP_delay(BSP_TICKS_PER_SEC * 3U / 4U);
    }
}

uint32_t stack_blinky2[40];
OSThread blinky2;
void main_blinky2() {
    while (1) {
        BSP_ledBlueOn();
        //BSP_delay(BSP_TICKS_PER_SEC / 2U);
        BSP_ledBlueOff();
        //BSP_delay(BSP_TICKS_PER_SEC / 3U);
    }
}

uint32_t stack_blinky3[40];
OSThread blinky3;
void main_blinky3() {
    while (1) {
        BSP_ledRedOn();
        //BSP_delay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        //BSP_delay(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}

int main() {
    BSP_init();
    OS_init();

    /* start blinky1 thread */
    OSThread_start(&blinky1,
                   &main_blinky1,
                   stack_blinky1, sizeof(stack_blinky1));

    /* start blinky2 thread */
    OSThread_start(&blinky2,
                   &main_blinky2,
                   stack_blinky2, sizeof(stack_blinky2));

    /* start blinky3 thread */
    OSThread_start(&blinky3,
                   &main_blinky3,
                   stack_blinky3, sizeof(stack_blinky3));

    /* transfer control to the RTOS to run the threads */
    OS_run();

    //return 0;
}
