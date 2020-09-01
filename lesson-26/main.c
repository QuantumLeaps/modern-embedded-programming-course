#include <stdint.h>
#include "miros.h"
#include "bsp.h"

uint32_t stack_blinky1[40];
OSThread blinky1;
void main_blinky1() {
    while (1) {
        uint32_t volatile i;
        for (i = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledGreenOff();
        }
        OS_delay(1U); /* block for 1 tick */
    }
}

uint32_t stack_blinky2[40];
OSThread blinky2;
void main_blinky2() {
    while (1) {
        uint32_t volatile i;
        for (i = 3*1500U; i != 0U; --i) {
            BSP_ledBlueOn();
            BSP_ledBlueOff();
        }
        OS_delay(50U); /* block for 50 ticks */
    }
}

uint32_t stack_blinky3[40];
OSThread blinky3;
void main_blinky3() {
    while (1) {
        BSP_ledRedOn();
        OS_delay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        OS_delay(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}

uint32_t stack_idleThread[40];

int main() {
    BSP_init();
    OS_init(stack_idleThread, sizeof(stack_idleThread));

    /* start blinky1 thread */
    OSThread_start(&blinky1,
                   5U, /* priority */
                   &main_blinky1,
                   stack_blinky1, sizeof(stack_blinky1));

    /* start blinky2 thread */
    OSThread_start(&blinky2,
                   2U, /* priority */
                   &main_blinky2,
                   stack_blinky2, sizeof(stack_blinky2));

    /* start blinky3 thread */
    //OSThread_start(&blinky3,
    //               1U, /* priority */
    //               &main_blinky3,
    //               stack_blinky3, sizeof(stack_blinky3));

    /* transfer control to the RTOS to run the threads */
    OS_run();

    //return 0;
}
