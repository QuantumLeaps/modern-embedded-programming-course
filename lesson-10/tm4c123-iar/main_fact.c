////////////////////////////////////////////////////////////////
// NOTE: this contains the code that intentionally CORRUPTS
// and eventually overflows the stack as explained in the
// Lesson-10 video.

#include "tm4c.h"   // the same as "lm4f120h5qr.h" in the video
#include "delay.h"

#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

unsigned fact(unsigned n);

int main(void) {
    unsigned volatile x;

    x = fact(0U);
    x = 2U + 3U*fact(1U);
    x = fact(9U); // <== corrupts and overflows the stack!

    SYSCTL_GPIOHBCTL_R |= (1U << 5); /* enable AHB for GPIOF */
    SYSCTL_RCGCGPIO_R |= (1U << 5);  /* enable clock for GPIOF */

    GPIO_PORTF_AHB_DIR_R |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIO_PORTF_AHB_DEN_R |= (LED_RED | LED_BLUE | LED_GREEN);

    GPIO_PORTF_AHB_DATA_BITS_R[LED_BLUE] = LED_BLUE;
    while (1) {
        GPIO_PORTF_AHB_DATA_BITS_R[LED_RED] = LED_RED;

        delay(1000000);

        GPIO_PORTF_AHB_DATA_BITS_R[LED_RED] = 0;

        delay(500000);
    }
    //return 0; // unreachable code
}

unsigned fact(unsigned n) {
// 0! = 1
// n! = n*(n-1)!  for n > 0
    unsigned foo[6];
    foo[n] = n;

    if (n == 0U) {
        return 1U;
    }
    else {
        return foo[n] * fact(n - 1U);
    }
}

////////////////////////////////////////////////////////////////
// NOTE: the file main_swap.c in the project directory contains
// the code tat uses the swap() function to illustrate the use
// of pointer parameters in Lesson-10 video.
