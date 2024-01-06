#include "tm4c_cmsis.h"  // CMSIS-compatible interface
#include "delay.h"

#include <stdint.h> // C99 standard integers

#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

typedef struct /* _packed */ {
    uint8_t y;
    uint16_t x;
} Point;

Point p1, p2;

typedef struct {
    Point top_left;
    Point bottom_right;
} Window;

typedef struct {
     Point corners[3];
} Triangle;

Window w, w2;
Triangle t;

int main(void) {
    Point *pp;
    Window *wp;

    p1.x = sizeof(Point);
    p1.y = 0xAAU;

    w.top_left.x = 1U;
    w.bottom_right.y = 2U;

    t.corners[0].x = 1U;
    t.corners[2].y = 2U;

    p2 = p1;
    w2 = w;

    pp = &p1;
    wp = &w2;

    (*pp).x = 1U;

    (*wp).top_left = *pp;

    pp->x = 1U;
    wp->top_left = *pp;

    SYSCTL->GPIOHSCTL |= (1U << 5); /* enable AHB for GPIOF */
    SYSCTL->RCGC2 |= (1U << 5);  /* enable clock for GPIOF */

    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    /* turn all LEDs off */
    GPIOF_AHB->DATA_Bits[LED_RED | LED_BLUE | LED_GREEN] = 0U;

    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
    while (1) {
        GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
        delay(500000);

        GPIOF_AHB->DATA_Bits[LED_RED] = 0;

        delay(250000);
    }
    //return 0; // unreachable code
}
