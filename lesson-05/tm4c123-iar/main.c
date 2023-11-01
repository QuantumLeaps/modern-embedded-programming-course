#include "tm4c.h"   // the same as "lm4f120h5qr.h" in the video
/*
#define SYSCTL_RCGCGPIO_R (*((unsigned int *)0x400FE608U))

#define GPIO_PORTF_BASE   0x40025000U
#define GPIO_PORTF_DIR_R  (*((unsigned int *)(GPIO_PORTF_BASE + 0x400U)))
#define GPIO_PORTF_DEN_R  (*((unsigned int *)(GPIO_PORTF_BASE + 0x51CU)))
#define GPIO_PORTF_DATA_R (*((unsigned int *)(GPIO_PORTF_BASE + 0x3FCU)))
*/

int main(void) {
    SYSCTL_RCGCGPIO_R = 0x20U;  // enable clock for GPIOF
    GPIO_PORTF_DIR_R = 0x0EU; // set pins 1,2, and 3 as outputs
    GPIO_PORTF_DEN_R = 0x0EU; // enable digital function on pins 1,2, and 3

    while (1) { // endless loop
        GPIO_PORTF_DATA_R = 0x02U; // turn the red-LED on

        int volatile counter = 0;
        while (counter < 1000000) {  // delay loop
            ++counter;
        }

        GPIO_PORTF_DATA_R = 0x00U; // turn the red-LED off
        counter = 0;
        while (counter < 1000000) {  // delay loop
            ++counter;
        }

    }
    //return 0; // unreachable code
}
