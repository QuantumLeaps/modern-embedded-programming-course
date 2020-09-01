#include "qpc.h"
#include "bsp.h"
#include "shape.h"
#include "rectangle.h"

#include <stdlib.h> /* for malloc() and free() */

Q_DEFINE_THIS_FILE

QXSemaphore SW1_sema;

uint32_t stack_blinky1[40];
QXThread blinky1;
void main_blinky1(QXThread * const me) {
    while (1) {
        BSP_sendMorseCode(0xA8EEE2A0U); /* "SOS" */
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
            //BSP_ledBlueOn();
            //BSP_ledBlueOff();
            BSP_ledBlueToggle();
        }
    }
}

uint32_t stack_blinky3[40];
QXThread blinky3;
void main_blinky3(QXThread * const me) {
    while (1) {
        BSP_sendMorseCode(0xE22A3800U); /* "TEST" */
        BSP_sendMorseCode(0xE22A3800U); /* "TEST" */
        QXThread_delay(5U);
    }
}

Shape s1; /* static allocation */
Rectangle r1; /* static allocation */
uint32_t a;

int main() {
    Shape s2; /* automatic allocation */
    Shape *ps3 = malloc(sizeof(Shape)); /* dynamic allocation */
    //Shape const *ps1 = &s1; /* const pointer to s1 */

    Shape_ctor(&s1, 1, 2);
    Shape_ctor(&s2, 3, 4);
    Shape_ctor(ps3, 5, 6);
    Rectangle_ctor(&r1, 1, 2, 15, 10);

    Shape_moveBy(&s1, 7, 8);
    Shape_moveBy(&s2, 9, 10);
    Shape_moveBy(ps3, -1, -2);
    //Shape_moveBy(ps1, -3, -4);

    Rectangle_draw(&r1);
    a = Rectangle_area(&r1);
    Shape_moveBy((Shape *)&r1, 7, 8);
    Q_ASSERT(Shape_distanceFrom((Shape *)&r1, (Shape *)&r1) == 0U);


    Q_ASSERT(Shape_distanceFrom(&s1, &s1) == 0U);
    Q_ASSERT(Shape_distanceFrom(&s1, &s2) ==
             Shape_distanceFrom(&s2, &s1));
    Q_ASSERT(Shape_distanceFrom(&s1, &s2) <=
             Shape_distanceFrom(&s1, ps3)
           + Shape_distanceFrom(ps3, &s2));

    free(ps3);

    QF_init();
    BSP_init();

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
    QXThread_ctor(&blinky3, &main_blinky3, 0);
    QXTHREAD_START(&blinky3,
                   1U, /* priority */
                   (void *)0, 0, /* message queue (not used) */
                   stack_blinky3, sizeof(stack_blinky3), /* stack */
                   (void *)0); /* extra parameter (not used) */

    /* transfer control to the RTOS to run the threads */
    return QF_run();
}
