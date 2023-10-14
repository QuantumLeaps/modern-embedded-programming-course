////////////////////////////////////////////////////////////////
// NOTE:
// This file contains code that intentionally CORRUPTS and
// eventually overflows the stack as explained in the
// Lesson-10 video.

#include "stm32c031.h"
#include "delay.h"

// LED marked "LD4" on the NUCLEO-C031C6 board
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

unsigned fact(unsigned n); // prototype

int main(void) {
    unsigned volatile x;

    x = fact(0U);
    x = 2U + 3U*fact(1U);
    x = fact(9U); // <== corrupts and overflows the stack!

    // enable GPIOA clock port for the LEDs
    RCC_IOPENR_R |= (1U << 0U);

    // NUCLEO-C031C6 board has LED LD4 on GPIOA pin LD4_PIN
    // and external LED LD5 on GPIO LD5_PIN
    // set the LED pins as push-pull output, no pull-up, pull-down
    GPIOA_MODER_R   &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));
    GPIOA_MODER_R   |=  ((1U << 2U*LD4_PIN) | (1U << 2U*LD5_PIN));
    GPIOA_OTYPER_R  &= ~((1U <<    LD4_PIN) | (1U <<    LD5_PIN));
    GPIOA_OSPEEDR_R &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));
    GPIOA_OSPEEDR_R |=  ((1U << 2U*LD4_PIN) | (1U << 2U*LD5_PIN));
    GPIOA_PUPDR_R   &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));

    GPIOA_BSRR_R = (1U << LD5_PIN); // turn LD5 on

    while (1) { // endless loop
        GPIOA_BSRR_R = (1U << LD4_PIN); // turn LD4 on

        delay(500000);

        GPIOA_BSRR_R = (1U << (LD4_PIN + 16U)); // turn LD4 off

        delay(250000);
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
// the code with the swap() function to illustrate the use of
// pointer parameters in Lesson-10 video.
// To use the other file, simply copy main_swap.c to main.c.
