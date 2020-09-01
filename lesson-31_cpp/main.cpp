#include "qpc.h"
#include "bsp.h"
#include "shape.h"
#include "rectangle.h"
#include "circle.h"

#include <stdlib.h> /* for malloc() and free() */

Q_DEFINE_THIS_FILE

QXSemaphore SW1_sema;

Shape s1(1, 2); /* static allocation */
Rectangle r1(1, 2, 15, 10); /* static allocation */
uint32_t a;

Circle c1(3, 4, 10);

uint32_t stack_blinky1[40];
QXThread blinky1;
void main_blinky1(QXThread * const me) {
    while (1) {
        BSP_sendMorseCode(0xA8EEE2A0U); /* "SOS" */

        s1.moveBy(7, 8);

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

            Q_ASSERT(s1.distanceFrom(&s1) == 0U);
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

int main() {
    Shape s2(3, 4); /* automatic allocation */
    Shape *ps3 = new Shape(5, 6); /* dynamic allocation */
    //Shape const *ps1 = &s1; /* const pointer to s1 */

    Shape *ps = &r1; /* upcasting! */

    r1.draw(); /* early binding */
    ps->draw(); /* late binding */
    a = ps->area();

    Shape const *graph[] = {
        &c1,
        &r1,
        ps3,
        (Shape *)0
    };
    drawGraph(graph);

    s1.moveBy(7, 8);
    s2.moveBy(9, 10);
    ps3->moveBy(-1, -2);
    //ps1->moveBy(-3, -4);

    r1.draw();
    a = r1.area();
    r1.moveBy(7, 8);
    Q_ASSERT(r1.distanceFrom(&r1) == 0U);

    Q_ASSERT(s1.distanceFrom(&s1) == 0U);
    Q_ASSERT(s1.distanceFrom(&s2) ==
             s2.distanceFrom(&s1));
    Q_ASSERT(s1.distanceFrom(&s2) <=
             s1.distanceFrom(ps3)
           + ps3->distanceFrom(&s2));

    delete ps3;

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
                   (QEvt *)0); /* extra parameter (not used) */

    /* initialize and start blinky2 thread */
    QXThread_ctor(&blinky2, &main_blinky2, 0);
    QXTHREAD_START(&blinky2,
                   2U, /* priority */
                   (void *)0, 0, /* message queue (not used) */
                   stack_blinky2, sizeof(stack_blinky2), /* stack */
                   (QEvt *)0); /* extra parameter (not used) */

    /* initialize and start blinky3 thread */
    QXThread_ctor(&blinky3, &main_blinky3, 0);
    QXTHREAD_START(&blinky3,
                   1U, /* priority */
                   (void *)0, 0, /* message queue (not used) */
                   stack_blinky3, sizeof(stack_blinky3), /* stack */
                   (QEvt *)0); /* extra parameter (not used) */

    /* transfer control to the RTOS to run the threads */
    return QF_run();
}
