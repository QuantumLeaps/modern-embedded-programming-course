#include "stm32c031.h"

// LED marked "LD4" on the NUCLEO-C031C6 board
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

void delay(int iter); // prototype

void delay(int iter) {
    int volatile counter = 0;
    while (counter < iter) {  // delay loop
        ++counter;
    }
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

    while (1) { // endless loop
        GPIOA_BSRR_R = (1U << LD4_PIN); // turn LD4 on

        delay(1000000);

        GPIOA_BSRR_R = (1U << (LD4_PIN + 16U)); // turn LD4 off
        delay(500000);

    }
    //return 0; // unreachable code
}
