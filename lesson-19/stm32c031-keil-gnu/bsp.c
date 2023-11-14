#include "bsp.h"
#include "stm32c031xx.h"  // CMSIS-compatible interface

// LED marked "LD4" on the NUCLEO-C031C6 board
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U


//............................................................................
void SysTick_Handler(void) {
    if (GPIOA->IDR & (1U << LD4_PIN)) { // is LD4 ON?
        GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LD4 on
    }
    else { // LD4 is OFF
        GPIOA->BSRR = (1U << LD4_PIN); // turn LD4 on
    }
}

//............................................................................
void BSP_init(void) {
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

    // configure the SysTick interrupt
    SysTick->LOAD = SystemCoreClock/2U - 1U; // 1/2 second
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 2U) | (1U << 1U) | 1U;

}
//............................................................................
void BSP_led4On(void) {
    GPIOA->BSRR = (1U << LD4_PIN); // turn LD4 on
}
//............................................................................
void BSP_led4Off(void) {
    GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LD4 on
}
//............................................................................
void BSP_led5On(void) {
    GPIOA->BSRR = (1U << LD5_PIN); // turn LD5 on
}
//............................................................................
void BSP_led5Off(void) {
    GPIOA->BSRR = (1U << (LD5_PIN + 16U)); // turn LD5 on
}
//............................................................................
void BSP_delay(int volatile iter) {
    while (iter > 0) {  // delay loop
        --iter;
    }
}

//............................................................................
void assert_failed(char const * const module, int const id); // prototype
void assert_failed(char const * const module, int const id) {
    (void)module; // unused parameter
    (void)id;     // unused parameter
#ifndef NDEBUG
    // light up the user LED
    GPIOA->BSRR = (1U << LD4_PIN);  // turn LED on
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif

    NVIC_SystemReset();
}
