#include "stm32c031xx.h"  // CMSIS-compatible interface
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
    (void)fact(5U);

    // enable GPIOA clock port for the LEDs
    RCC->IOPENR |= (1U << 0U);

    // NUCLEO-C031C6 board has LED LD4 on GPIOA pin LD4_PIN
    // and external LED LD5 on GPIO LD5_PIN
    // set the LED pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));
    GPIOA->MODER   |=  ((1U << 2U*LD4_PIN) | (1U << 2U*LD5_PIN));
    GPIOA->OTYPER  &= ~((1U <<    LD4_PIN) | (1U <<    LD5_PIN));
    GPIOA->OSPEEDR &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));
    GPIOA->OSPEEDR |=  ((1U << 2U*LD4_PIN) | (1U << 2U*LD5_PIN));
    GPIOA->PUPDR   &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));

    GPIOA->BSRR = (1U << LD5_PIN); // turn LD5 on

    while (1) { // endless loop
        GPIOA->BSRR = (1U << LD4_PIN); // turn LD4 on

        delay(500000);

        GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LD4 off
        delay(250000);

    }
    //return 0; // unreachable code
}

unsigned fact(unsigned n) {
// 0! = 1
// n! = n*(n-1)!  for n > 0

    if (n == 0U) {
        return 1U;
    }
    else {
        return n * fact(n - 1U);
    }
}

