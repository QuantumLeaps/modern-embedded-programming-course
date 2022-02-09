#include "lm4f120h5qr.h"

#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

int main() {

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

    SYSCTL_RCGCGPIO_R |= (1U << 5);  /* enable clock for GPIOF */
    GPIO_PORTF_DIR_R |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIO_PORTF_DEN_R |= (LED_RED | LED_BLUE | LED_GREEN);

    /* start with turning all LEDs off */
    GPIO_PORTF_DATA_R &= ~(LED_RED | LED_BLUE | LED_GREEN);

    GPIO_PORTF_DATA_R |= LED_BLUE;
    while (1) {
        GPIO_PORTF_DATA_R |= LED_RED;

        int volatile counter = 0;
        while (counter < 1000000) {  // delay loop
            ++counter;
        }

        GPIO_PORTF_DATA_R &= ~LED_RED;
        counter = 0;
        while (counter < 1000000) {  // delay loop
            ++counter;
        }

    }
    //return 0;
}
