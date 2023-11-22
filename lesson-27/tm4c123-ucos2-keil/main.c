#include "ucos_ii.h"    /* uC-OS2 API */
#include <stdint.h>     /* C99 standard integers */
#include "bsp.h"        /* Board Support Package */

_Noreturn void assert_failed(char const * const module, int const id);

static OS_STK stack_blinky1[100];
void main_blinky1(void *pdata) {
    while (1) {
        uint32_t volatile i;
        for (i = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledGreenOff();
        }
        OSTimeDly(1U); /* block for 1 tick */
    }
}

static OS_STK stack_blinky2[100];
void main_blinky2(void *pdata) {
    while (1) {
        INT8U err;
        OSSemPend(SW1_sema, 0, &err);

        for (uint32_t volatile i = 3*1500U; i != 0U; --i) {
            BSP_ledBlueOn();
            BSP_ledBlueOff();
        }
    }
}

static OS_STK stack_blinky3[100];
void main_blinky3(void *pdata) {
    while (1) {
        BSP_ledRedOn();
        OSTimeDly(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        OSTimeDly(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}

int main() {
    BSP_init(); /* initialize the Board Support Package */
    OSInit();   /* initialize uC/OS-II */

    /* instantiate and start all FreeRTOS tasks and queues... */
    INT8U err;

    /* initialize the SW1_sema semaphore as binary, signaling semaphore */
    SW1_sema = OSSemCreate(0);
    if (SW1_sema == (OS_EVENT *)0) {
        assert_failed(__FILE__, __LINE__);
    }

    /* initialize and start blinky1 task... */
    err = OSTaskCreate(
        &main_blinky1,     /* task function */
        (void *)0,         /* pdata */
#if OS_STK_GROWTH
        &stack_blinky1[sizeof(stack_blinky1)/sizeof(stack_blinky1[0])],
#else
        &stack_blinky1[0], /* ptos */
#endif
        OS_LOWEST_PRIO - 5U); /* task priority (reversed order!) */
    if (err != 0) {
        assert_failed(__FILE__, __LINE__);
    }

    /* initialize and start blinky2 task... */
    err = OSTaskCreate(
        &main_blinky2,     /* task function */
        (void *)0,         /* pdata */
#if OS_STK_GROWTH
        &stack_blinky2[sizeof(stack_blinky2)/sizeof(stack_blinky2[0])],
#else
        &stack_blinky2[0], /* ptos */
#endif
        OS_LOWEST_PRIO - 3U); /* task priority (reversed order!) */
    if (err != 0) {
        assert_failed(__FILE__, __LINE__);
    }

    /* initialize and start blinky3 task... */
//    err = OSTaskCreate(
//        &main_blinky3,     /* task function */
//        (void *)0,         /* pdata */
//#if OS_STK_GROWTH
//        &stack_blinky3[sizeof(stack_blinky3)/sizeof(stack_blinky3[0])],
//#else
//        &stack_blinky3[0], /* ptos */
//#endif
//        OS_LOWEST_PRIO - 1U); /* task priority (reversed order!) */
//    if (err != 0) {
//        assert_failed(__FILE__, __LINE__);
//    }

    BSP_onStartup();

    OSStart();       /* start uC-OS2 multitasking */
    return 0; /* NOTE: the scheduler does NOT return */
}
