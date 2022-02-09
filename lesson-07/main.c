#include "lm4f120h5qr.h"
#include <intrinsics.h>

#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

int main() {
    SYSCTL_RCGCGPIO_R |= (1U << 5);  /* enable clock for GPIOF */
    SYSCTL_GPIOHBCTL_R |= (1U << 5); /* enable AHB for GPIOF */

    /* make sure the Run Mode and AHB-enable take effects
    * before accessing the peripherals
    */
    __ISB(); /* Instruction Synchronization Barrier */
    __DSB(); /* Data Memory Barrier */

    GPIO_PORTF_AHB_DIR_R |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIO_PORTF_AHB_DEN_R |= (LED_RED | LED_BLUE | LED_GREEN);

    /* start with turning all LEDs off */
    GPIO_PORTF_AHB_DATA_BITS_R[LED_RED | LED_BLUE | LED_GREEN] = 0;

    GPIO_PORTF_AHB_DATA_BITS_R[LED_BLUE] = LED_BLUE;
    while (1) {
        //*((unsigned long volatile *)(0x40025000 + (LED_RED << 2))) = LED_RED;
        //*(GPIO_PORTF_DATA_BITS_R + LED_RED) = LED_RED;
        GPIO_PORTF_AHB_DATA_BITS_R[LED_RED] = LED_RED;

        int volatile counter;
        counter = 0;
        while (counter < 1000000) {  // delay loop
            ++counter;
        }

        GPIO_PORTF_AHB_DATA_BITS_R[LED_RED] = 0;
        counter = 0;
        while (counter < 1000000) {  // delay loop
            ++counter;
        }

    }
    //return 0;
}
