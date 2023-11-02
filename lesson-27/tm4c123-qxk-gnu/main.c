#include "qpc.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

QXSemaphore SW1_sema;

uint32_t stack_blinky1[40];
QXThread blinky1;
void main_blinky1(QXThread * const me) {
    while (1) {
        uint32_t volatile i;
        for (i = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledGreenOff();
        }
        QXThread_delay(1U); /* block for 1 tick */
    }
}

uint32_t stack_blinky2[40];
QXThread blinky2;
void main_blinky2(QXThread * const me) {
    while (1) {
        uint32_t volatile i;

        QXSemaphore_wait(&SW1_sema,  /* pointer to semaphore to wait on */
                         QXTHREAD_NO_TIMEOUT); /* timeout for waiting */

        for (i = 3*1500U; i != 0U; --i) {
            BSP_ledBlueOn();
            BSP_ledBlueOff();
        }
    }
}

uint32_t stack_blinky3[40];
QXThread blinky3;
void main_blinky3(QXThread * const me) {
    while (1) {
        BSP_ledRedOn();
        QXThread_delay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        QXThread_delay(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}

int main() {
    BSP_init();
    QF_init();

    /* initialize the SW1_sema semaphore as binary, signaling semaphore */
    QXSemaphore_init(&SW1_sema, /* pointer to semaphore to initialize */
                     0U,  /* initial semaphore count (singaling semaphore) */
                     1U); /* maximum semaphore count (binary semaphore) */

    /* initialize and start blinky1 thread */
    QXThread_ctor(&blinky1, &main_blinky1, 0);
    QXTHREAD_START(&blinky1,
                   5U, /* priority */
                   (void *)0, 0, /* message queue (not used) */
                   stack_blinky1, sizeof(stack_blinky1), /* stack */
                   (void *)0); /* extra parameter (not used) */

    /* initialize and start blinky2 thread */
    QXThread_ctor(&blinky2, &main_blinky2, 0);
    QXTHREAD_START(&blinky2,
                   2U, /* priority */
                   (void *)0, 0, /* message queue (not used) */
                   stack_blinky2, sizeof(stack_blinky2), /* stack */
                   (void *)0); /* extra parameter (not used) */

    /* initialize and start blinky3 thread */
    //QXThread_ctor(&blinky3, &main_blinky3, 0);
    //QXTHREAD_START(&blinky3,
    //               1U, /* priority */
    //               (void *)0, 0, /* message queue (not used) */
    //               stack_blinky3, sizeof(stack_blinky3), /* stack */
    //               (void *)0); /* extra parameter (not used) */

    /* transfer control to the RTOS to run the threads */
    return QF_run();
}
