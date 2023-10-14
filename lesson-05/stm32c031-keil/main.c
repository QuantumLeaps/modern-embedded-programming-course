#include "stm32c031.h"

// NOTE:
// NUCLEO-C031C6 registers require using the binary operators
// such as '|', '&', and '~' that will be explained in the
// next lesson-06

int main(void) {

    // enable GPIOA clock port for the LED LD4
    RCC_IOPENR_R |= 0x01U;

    // NUCLEO-C031C6 board has user LED LD4 on GPIOA pin 5
    // set the LED pin as push-pull output, no pull-up, pull-down
    GPIOA_MODER_R   &= ~0xC00U;
    GPIOA_MODER_R   |=  0x400U;
    GPIOA_OTYPER_R  &= ~0x020U;
    GPIOA_OSPEEDR_R &= ~0xC00U;
    GPIOA_OSPEEDR_R |=  0x400U;
    GPIOA_PUPDR_R   &= ~0xC00U;


    while (1) { // endless loop
        GPIOA_BSRR_R = 0x20U; // turn the LED on

        int volatile counter = 0;
        while (counter < 500000) {  // delay loop
            ++counter;
        }

        GPIOA_BSRR_R = 0x200000U;  // turn the LED off
        counter = 0;
        while (counter < 500000) {  // delay loop
            ++counter;
        }

    }
    //return 0; // unreachable code
}
