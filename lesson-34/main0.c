/* Blinky/Button with uC/OS-II RTOS */
#include "ucos_ii.h"  /* uC/OS-II API, port and compile-time configuration */
#include "bsp.h"

static char const this_module[] = "main"; /* this module name for BSP_ASSERT() */

/* The Blinky thread =========================================================*/
OS_STK stack_blinky[100]; /* task stack */

enum { INITIAL_BLINK_TIME = (OS_TICKS_PER_SEC / 4) };

/* data shared between tasks */
INT32U volatile shared_blink_time = INITIAL_BLINK_TIME;
OS_EVENT *shared_blink_time_mutex;

void main_blinky(void *pdata) { /* task function */
    (void)pdata; /* unused parameter(s) */

    while (1) { /* endless "superloop" */
        INT8U err;
        INT32U bt; /* local copy of shared_blink_time */

        OSMutexPend(shared_blink_time_mutex, 0, &err); /* mutual exclusion */
        BSP_ASSERT(err == 0);
        bt = shared_blink_time;
        OSMutexPost(shared_blink_time_mutex); /* mutual exclusion */

        BSP_ledGreenOn();
        OSTimeDly(bt);       /* BLOCKING! */
        BSP_ledGreenOff();
        OSTimeDly(bt * 3U);  /* BLOCKING! */
    }
}

/* The Button thread =========================================================*/
OS_STK stack_button[100]; /* task stack */

void main_button(void *pdata) { /* task function */
    (void)pdata; /* unused parameter(s) */

    while (1) { /* endless "superloop" */
        INT8U err; /* uC/OS-II error status */

        /* wait on the button-press semaphore (BLOCK indefinitely) */
        OSSemPend(BSP_semaPress, 0, &err); /* BLOCKING! */
        BSP_ASSERT(err == 0);
        BSP_ledBlueOn();

        /* update the blink time for the 'blink' thread */
        OSMutexPend(shared_blink_time_mutex, 0, &err); /* mutual exclusion */
        BSP_ASSERT(err == 0);
        shared_blink_time >>= 1; /* shorten the blink time by factor of 2 */
        if (shared_blink_time == 0U) {
            shared_blink_time = INITIAL_BLINK_TIME;
        }
        OSMutexPost(shared_blink_time_mutex); /* mutual exclusion */

        /* wait on the button-release semaphore (BLOCK indefinitely) */
        OSSemPend(BSP_semaRelease, 0, &err); /* BLOCKING! */
        BSP_ledBlueOff();
    }
}

OS_EVENT *BSP_semaPress;   /* global semaphore handle */
OS_EVENT *BSP_semaRelease; /* global semaphore handle */

/* the main function =========================================================*/
int main() {
    INT8U err;

    BSP_init(); /* initialize the BSP */
    OSInit();   /* initialize uC/OS-II */

    /* initialize the RTOS objects before using them */
    BSP_semaPress   = OSSemCreate(0);
    BSP_ASSERT(BSP_semaPress);
    BSP_semaRelease = OSSemCreate(0);
    BSP_ASSERT(BSP_semaRelease);
    shared_blink_time_mutex = OSMutexCreate(OS_LOWEST_PRIO - 5, &err);
    BSP_ASSERT(err == 0);

    /* create uC/OS-II task, see NOTE1 */
    err = OSTaskCreateExt(&main_blinky, /* the task function */
          (void *)0,      /* the 'pdata' parameter (not used) */
          &stack_blinky[(sizeof(stack_blinky) / sizeof(OS_STK)) - 1], /* ptos */
          OS_LOWEST_PRIO - 4, /* uC/OS-II task priority */
          OS_LOWEST_PRIO - 4, /* unique priority is used as the task ID */
          stack_blinky,   /* pbos */
          (INT32U)(sizeof(stack_blinky)/sizeof(OS_STK)),/* stack depth */
          (void *)0,      /* pext */
          (INT16U)0);     /* task options */
    BSP_ASSERT(err == 0);

    /* create uC/OS-II task, see NOTE1 */
    err = OSTaskCreateExt(&main_button, /* the task function */
          (void *)0,      /* the 'pdata' parameter (not used) */
          &stack_button[(sizeof(stack_button) / sizeof(OS_STK)) - 1], /* ptos */
          OS_LOWEST_PRIO - 3, /* uC/OS-II task priority */
          OS_LOWEST_PRIO - 3, /* unique priority is used as the task ID */
          stack_button,   /* pbos */
          (INT32U)(sizeof(stack_button)/sizeof(OS_STK)),/* stack depth */
          (void *)0,      /* pext */
          (INT16U)0);     /* task options */
    BSP_ASSERT(err == 0);

    BSP_start(); /* configure and start the interrupts */

    OSStart(); /* start the uC/OS-II scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}

/*******************************************************************************
* NOTE1:
* The call to uC/OS-II API OSTaskCreateExt() assumes that the pointer to the
* top-of-stack (ptos) is at the end of the provided stack memory. This is
* correct only for CPUs with downward-growing stack, but must be changed for
* CPUs with upward-growing stack.
*/
