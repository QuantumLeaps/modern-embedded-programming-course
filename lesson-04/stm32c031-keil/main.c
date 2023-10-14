// NOTE:
// NUCLEO-C031C6 registers require using the binary operators
// such as '|', '&', and '~' that will be explained in lesson-06

int main(void) {

    // enable GPIOA clock port for the LED LD4
    *((unsigned int *)0x40021034U) |= 0x01U; // RCC IOPENR register

    // NUCLEO-C031C6 board has user LED LD4 on GPIOA pin 5
    // set the LED pin as push-pull output, no pull-up, pull-down
    *((unsigned int *)0x50000000U) &= ~0xC00U; // GPIOA MODER register
    *((unsigned int *)0x50000000U) |=  0x400U; // GPIOA MODER register
    *((unsigned int *)0x50000004U) &= ~0x020U; // GPIOA OTYPER register
    *((unsigned int *)0x50000008U) &= ~0xC00U; // GPIOA OSPEEDR register
    *((unsigned int *)0x50000008U) |=  0x400U; // GPIOA OSPEEDR register
    *((unsigned int *)0x5000000CU) &= ~0xC00U; // GPIOA PUPDR register


    while (1) { // endless loop
        *((unsigned int *)0x50000018U) = 0x20U; // GPIOA BSRR register

        int volatile counter = 0;
        while (counter < 500000) {  // delay loop
            ++counter;
        }

        *((unsigned int *)0x50000018U) = 0x200000U; // GPIOA BSRR register
        counter = 0;
        while (counter < 500000) {  // delay loop
            ++counter;
        }

    }
    //return 0; // unreachable code
}
