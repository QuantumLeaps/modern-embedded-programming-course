#include "stm32c031.h"
#include "delay.h"

#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

int *swap(int *x, int *y);
int *swap(int *x, int *y) {
    static int tmp[2];
    tmp[0] = *x;
    tmp[1] = *y;
    *x = tmp[1];
    *y = tmp[0];
    return tmp;
}

int main(void) {

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

    int x = 1000000;
    int y = 1000000/2;
    while (1) { // endless loop
        int *p = swap(&x, &y);

        GPIOA_BSRR_R = (1U << LD4_PIN); // turn LD4 on
        delay(p[0]);

        GPIOA_BSRR_R = (1U << (LD4_PIN + 16U)); // turn LD4 off

        delay(p[1]);
    }
    //return 0; // unreachable code
}

////////////////////////////////////////////////////////////////
// NOTE: the file main_swap.c in the project directory contains
// the code with the swap() function to illustrate the use of
// pointer parameters in Lesson-10 video.
// To use the other file, simply copy main_swap.c to main.c.
