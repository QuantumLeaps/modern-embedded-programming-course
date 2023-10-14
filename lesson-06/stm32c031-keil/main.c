#include "stm32c031.h"

// LED marked "LD4" on the NUCLEO-C031C6 board
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

int main(void) {

/*
    unsigned int a = 0x5A5A5A5A;
    unsigned int b = 0xDEADBEEF;
    unsigned int c;

    c = a | b;   // OR
    c = a & b;   // AND
    c = a ^ b;   // XOR
    c = ~b;      // NOT
    c = a << 1;  // right-shift
    c = a << 2;
    c = b >> 1;  // left-shift
    c = b >> 3;

    int x = 1024;
    int y = -1024;
    int z;

    z = x >> 3;
    z = y >> 3;
*/

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

        int volatile counter = 0;
        while (counter < 500000) {  // delay loop
            ++counter;
        }

        GPIOA_BSRR_R = (1U << (LD4_PIN + 16U)); // turn LD4 off
        counter = 0;
        while (counter < 500000) {  // delay loop
            ++counter;
        }

    }
    //return 0; // unreachable code
}
