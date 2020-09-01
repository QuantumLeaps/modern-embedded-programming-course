#include <stdint.h>

#include "tm4c_cmsis.h"
#include "delay.h"

#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

typedef struct {
    uint8_t y;
    uint16_t x;
} Point;

Point p1 = {
    0x12U,
    0xABCDU
};
Point p2/*[] = {
  { 0x10U, 0xA010U},
  { 0x11U, 0xA111U}
}*/;

typedef struct {
    Point top_left;
    Point bottom_right;
} Window;

typedef struct {
     Point corners[3];
} Triangle;

Window w, w2;
Triangle const t = {
  {
    { 1, 2},
    { 1, 3},
    { 1, 4}
  }
};

int main() {
    Point *pp;
    Window *wp;

    volatile uint32_t x = ~0U;
    volatile uint32_t y = CLZ6(x);

    p1.x = sizeof(Point);
    p1.y = sizeof(p1.x);

    w.top_left.x = 1U;
    w.bottom_right.y = 2U;

    //t.corners[0].x = 1U;
    //t.corners[2].y = 2U;

    p2 = t.corners[0];
    w2 = w;

    //pp = (Point *)0x10000000;
    pp = &p1;
    wp = &w2;

    (*pp).x = 1U;

    (*wp).top_left = *pp;

    pp->x = 1U;
    wp->top_left = *pp;

    SYSCTL->RCGC2 |= (1U << 5);  /* enable clock for GPIOF */
    SYSCTL->GPIOHSCTL |= (1U << 5); /* enable AHB for GPIOF */
    GPIOF_HS->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_HS->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    GPIOF_HS->DATA_Bits[LED_BLUE] = LED_BLUE;
    while (1) {
        GPIOF_HS->DATA_Bits[LED_RED] = LED_RED;
        delay(500000);

        GPIOF_HS->DATA_Bits[LED_RED] = 0;

        delay(500000);
    }
    //return 0;
}

void assert_failed (char const *file, int line) {
  /* log the problem in non-volatile memory... */
  NVIC_SystemReset(); /* reset the system! */
}

